#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

#include <libultraship/libultraship.h>
#include <libultraship/bridge.h>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/cvar_prefixes.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "core1/music.h"
#include "core1/musicplayer.h"
extern CoMusic* comusicTracks;
void port_spaceworldMusicRefreshPhysicalMusicSlotVolume(int32_t index);
}

#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#define CVAR_SPACEWORLD_MUSIC CVAR_ENHANCEMENT("Restorations.SpaceworldMusic")

namespace {

enum SpaceworldVariant {
    SPACEWORLD_START = 0,
    SPACEWORLD_VILLAGE = 1,
    SPACEWORLD_TOWER = 2,
    SPACEWORLD_UNDERWATER = 3,
};

struct DecodedTrack {
    std::vector<int16_t> samples;
    uint32_t channels = 0;
    uint32_t sampleRate = 0;
    uint64_t frameCount = 0;

    bool Ready() const {
        return !samples.empty() && channels != 0 && sampleRate != 0 && frameCount != 0;
    }
};

DecodedTrack sStartTrack;
DecodedTrack sVillageTrack;
DecodedTrack sTowerTrack;
DecodedTrack sUnderwaterTrack;

std::atomic<bool> sStartReady{ false };
std::atomic<bool> sVillageReady{ false };
std::atomic<bool> sTowerReady{ false };
std::atomic<bool> sUnderwaterReady{ false };
std::atomic<bool> sMixActive{ false };
std::atomic<bool> sResetPlayback{ true };
std::atomic<int> sDesiredVariant{ SPACEWORLD_START };
std::atomic<float> sPrimaryMusicGain{ 1.0f };
std::atomic<bool> sIgnorePrimaryMusicHook{ false };

// ------------------------------------------------------------------
// Retail music-system bridge.
//
// The normal Banjo music system remains authoritative. The external
// Spaceworld recordings mirror its primary slot instead of inventing
// separate rules for pause/fanfares/area changes.
// ------------------------------------------------------------------
constexpr bool sRetailMusicSystemBridgeInstalled = true;

std::atomic<int32_t> sRetailPrimaryTrack{ -1 };
std::atomic<bool> sRetailPrimaryPlaying{ false };
std::atomic<int32_t> sRetailPrimaryChannelMask{ 0 };
std::atomic<float> sRetailVariantTransitionSeconds{ 1.5f };

SpaceworldVariant VariantFromRetailChannelMask(int32_t channelMask) {
    // Follow retail Mumbo's Mountain's own outdoor arrangement masks.
    // 0xB0C0 is the Village/Mumbo's Hut area.
    // 0x513F is the exterior of Ticker's Tower / termite mound.
    if (channelMask == 0xB0C0) {
        return SPACEWORLD_VILLAGE;
    }
    if (channelMask == 0x513F) {
        return SPACEWORLD_TOWER;
    }
    if (channelMask == 0x0200) {
        return SPACEWORLD_UNDERWATER;
    }
    return SPACEWORLD_START;
}


bool sLoadAttempted = false;

// Shared musical timeline. Start and Village are synchronized recordings, so
// changing variants never restarts the composition.
double sTimelineSeconds = 0.0;



// Runtime-measured relationship between the two decoded recordings.
// VillageTime = StartTime * sVillageSyncScale + sVillageSyncOffsetSeconds.
double sVillageSyncScale = 1.0;
double sVillageSyncOffsetSeconds = 0.0;
bool sVillageSyncCalibrated = false;

// TowerTime = StartTime * sTowerSyncScale + sTowerSyncOffsetSeconds.
double sTowerSyncScale = 1.0;
double sTowerSyncOffsetSeconds = 0.0;
bool sTowerSyncCalibrated = false;

// Underwater uses the same musical composition but the supplied recording
// runs very slightly faster. Its scale/offset are pinned to its own clean
// hard-loop seam so surfacing never changes musical phase.
double sUnderwaterSyncScale = 1.0;
double sUnderwaterSyncOffsetSeconds = 0.0;
bool sUnderwaterSyncCalibrated = false;

// Arrangement weights are driven by retail's own channel-mask transitions.
double sVillageBlend = 0.0;
double sTowerBlend = 0.0;
double sUnderwaterBlend = 0.0;
constexpr double kOutputSampleRate = 22000.0;
constexpr float kMusicGain = 0.73f;

// Shared Start/Village loop seam, aligned to the external 22 kHz mixer.
double sLoopStartSeconds = 0.5596825396825397;
double sLoopEndSeconds = 128.55619047619047;
double sLoopLengthSeconds = 127.99650793650793;
// Retail channel masks fade between area arrangements instead of snapping.
// A short recording crossfade gives the external tracks the same feel.
constexpr double kVariantCrossfadeSeconds = 1.5;

bool LoadMp3(const std::string& path, DecodedTrack& out) {
    drmp3_config config{};
    drmp3_uint64 totalFrames = 0;
    drmp3_int16* pcm =
        drmp3_open_file_and_read_pcm_frames_s16(path.c_str(), &config, &totalFrames, nullptr);

    if (pcm == nullptr || totalFrames == 0 || config.channels == 0 || config.sampleRate == 0) {
        if (pcm != nullptr) {
            drmp3_free(pcm, nullptr);
        }
        return false;
    }

    const uint64_t sampleCount64 = totalFrames * config.channels;
    if (sampleCount64 > static_cast<uint64_t>(SIZE_MAX)) {
        drmp3_free(pcm, nullptr);
        return false;
    }

    out.samples.assign(pcm, pcm + static_cast<size_t>(sampleCount64));
    out.channels = config.channels;
    out.sampleRate = config.sampleRate;
    out.frameCount = totalFrames;
    drmp3_free(pcm, nullptr);
    return true;
}

bool LoadFlac(const std::string& path, DecodedTrack& out) {
    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    drflac_uint64 totalFrames = 0;

    drflac_int16* pcm = drflac_open_file_and_read_pcm_frames_s16(
        path.c_str(), &channels, &sampleRate, &totalFrames, nullptr);

    if (pcm == nullptr || totalFrames == 0 || channels == 0 || sampleRate == 0) {
        if (pcm != nullptr) {
            drflac_free(pcm, nullptr);
        }
        return false;
    }

    const uint64_t sampleCount64 = totalFrames * channels;
    if (sampleCount64 > static_cast<uint64_t>(SIZE_MAX)) {
        drflac_free(pcm, nullptr);
        return false;
    }

    out.samples.assign(pcm, pcm + static_cast<size_t>(sampleCount64));
    out.channels = channels;
    out.sampleRate = sampleRate;
    out.frameCount = totalFrames;
    drflac_free(pcm, nullptr);
    return true;
}



// Find the cleanest hard seam from the exact dr_mp3-decoded PCM that Lighthouse
// will actually play. Search is intentionally limited to +/-10 ms around the
// already listening-approved timing so musical timing cannot wander.
double ReadLoopSearchSample(const DecodedTrack& track, int64_t outputFrame, uint32_t channel) {
    if (!track.Ready() || outputFrame < 0) {
        return 0.0;
    }

    const double seconds =
        static_cast<double>(outputFrame) / kOutputSampleRate;
    double framePosition =
        seconds * static_cast<double>(track.sampleRate);

    if (framePosition < 0.0) {
        framePosition = 0.0;
    }

    const double maxFrame =
        static_cast<double>(track.frameCount - 1);
    if (framePosition > maxFrame) {
        framePosition = maxFrame;
    }

    const uint64_t frame0 =
        static_cast<uint64_t>(framePosition);
    const uint64_t frame1 =
        (frame0 + 1 < track.frameCount) ? frame0 + 1 : frame0;
    const double fraction =
        framePosition - static_cast<double>(frame0);

    uint32_t actualChannel = channel;
    if (track.channels == 1) {
        actualChannel = 0;
    } else if (actualChannel >= track.channels) {
        actualChannel = track.channels - 1;
    }

    const double a = static_cast<double>(
        track.samples[frame0 * track.channels + actualChannel]);
    const double b = static_cast<double>(
        track.samples[frame1 * track.channels + actualChannel]);

    return a + (b - a) * fraction;
}

double ScoreHardLoopCandidate(const DecodedTrack& track,
                              int64_t startFrame,
                              int64_t endFrame) {
    double score = 0.0;

    for (uint32_t channel = 0;
         channel < std::min<uint32_t>(track.channels, 2);
         channel++) {
        const double e0 = ReadLoopSearchSample(track, endFrame - 1, channel);
        const double e1 = ReadLoopSearchSample(track, endFrame - 2, channel);
        const double e2 = ReadLoopSearchSample(track, endFrame - 3, channel);

        const double s0 = ReadLoopSearchSample(track, startFrame, channel);
        const double s1 = ReadLoopSearchSample(track, startFrame + 1, channel);
        const double s2 = ReadLoopSearchSample(track, startFrame + 2, channel);

        const double jump = s0 - e0;
        const double tailSlope = e0 - e1;
        const double headSlope = s1 - s0;
        const double slopeChange = headSlope - tailSlope;

        const double tailCurve = e0 - 2.0 * e1 + e2;
        const double headCurve = s2 - 2.0 * s1 + s0;
        const double curveChange = headCurve - tailCurve;

        score += (jump / 150.0) * (jump / 150.0);
        score += (slopeChange / 120.0) * (slopeChange / 120.0);
        score += (curveChange / 180.0) * (curveChange / 180.0);

        double preSq1 = 0.0;
        double postSq1 = 0.0;
        double preSq2 = 0.0;
        double postSq2 = 0.0;

        constexpr int kWindow1 = 22;
        constexpr int kWindow2 = 44;

        for (int i = 0; i < kWindow2; i++) {
            const double pre =
                ReadLoopSearchSample(track, endFrame - 1 - i, channel);
            const double post =
                ReadLoopSearchSample(track, startFrame + i, channel);

            if (i < kWindow1) {
                preSq1 += pre * pre;
                postSq1 += post * post;
            }

            preSq2 += pre * pre;
            postSq2 += post * post;
        }

        const double preRms1 = std::sqrt(preSq1 / kWindow1);
        const double postRms1 = std::sqrt(postSq1 / kWindow1);
        const double preRms2 = std::sqrt(preSq2 / kWindow2);
        const double postRms2 = std::sqrt(postSq2 / kWindow2);

        const double rmsDiff1 = postRms1 - preRms1;
        const double rmsDiff2 = postRms2 - preRms2;

        score += (rmsDiff1 / 500.0) * (rmsDiff1 / 500.0);
        score += 0.4 * (rmsDiff2 / 800.0) * (rmsDiff2 / 800.0);
    }

    return score;
}

void CalibrateExactDecodedHardLoop() {
    if (!sStartTrack.Ready()) {
        return;
    }

    constexpr double kCenterStartSeconds = 0.5596825396825397;
    constexpr double kCenterEndSeconds = 128.55619047619047;
    constexpr int64_t kSearchRadiusFrames = 220;

    const int64_t centerStart =
        static_cast<int64_t>(std::llround(
            kCenterStartSeconds * kOutputSampleRate));
    const int64_t centerEnd =
        static_cast<int64_t>(std::llround(
            kCenterEndSeconds * kOutputSampleRate));

    double bestScore = 1.0e300;
    int64_t bestStart = centerStart;
    int64_t bestEnd = centerEnd;

    for (int64_t startFrame = centerStart - kSearchRadiusFrames;
         startFrame <= centerStart + kSearchRadiusFrames;
         startFrame++) {
        for (int64_t endFrame = centerEnd - kSearchRadiusFrames;
             endFrame <= centerEnd + kSearchRadiusFrames;
             endFrame++) {
            if (endFrame <= startFrame + 1) {
                continue;
            }

            const double score =
                ScoreHardLoopCandidate(sStartTrack, startFrame, endFrame);

            if (score < bestScore) {
                bestScore = score;
                bestStart = startFrame;
                bestEnd = endFrame;
            }
        }
    }

    sLoopStartSeconds =
        static_cast<double>(bestStart) / kOutputSampleRate;
    sLoopEndSeconds =
        static_cast<double>(bestEnd) / kOutputSampleRate;
    sLoopLengthSeconds =
        static_cast<double>(bestEnd - bestStart) / kOutputSampleRate;
}


bool LoadStartTrack() {
    const std::string simplePath =
        Ship::Context::GetPathRelativeToAppDirectory("mods/spaceworld_music/Start.mp3");
    const std::string originalNamePath =
        Ship::Context::GetPathRelativeToAppDirectory(
            "mods/spaceworld_music/21. Mumbo Mountain - Start (Spaceworld).mp3");

    std::string path;
    if (!simplePath.empty() && std::filesystem::is_regular_file(simplePath)) {
        path = simplePath;
    } else if (!originalNamePath.empty() && std::filesystem::is_regular_file(originalNamePath)) {
        path = originalNamePath;
    } else {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Missing Start.mp3; using retail Mumbo's Mountain music.\n");
        return false;
    }

    if (!LoadMp3(path, sStartTrack)) {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Could not decode Start track; using retail music.\n");
        return false;
    }
    CalibrateExactDecodedHardLoop();
    sStartReady.store(true, std::memory_order_release);
    std::fprintf(stderr,
                 "[SpaceworldMusic] Loaded Start: %u Hz, %u channels, %.2f seconds.\n",
                 sStartTrack.sampleRate, sStartTrack.channels,
                 static_cast<double>(sStartTrack.frameCount) / sStartTrack.sampleRate);
    return true;
}

bool LoadVillageTrack() {
    const std::string flacPath =
        Ship::Context::GetPathRelativeToAppDirectory("mods/spaceworld_music/Village.flac");
    const std::string mp3Path =
        Ship::Context::GetPathRelativeToAppDirectory("mods/spaceworld_music/Village.mp3");
    const std::string originalFlacPath =
        Ship::Context::GetPathRelativeToAppDirectory(
            "mods/spaceworld_music/16. Mumbo Mountain - Village (Spaceworld).flac");
    const std::string originalMp3Path =
        Ship::Context::GetPathRelativeToAppDirectory(
            "mods/spaceworld_music/16. Mumbo Mountain - Village (Spaceworld).mp3");

    bool loaded = false;
    std::string usedPath;

    if (!flacPath.empty() && std::filesystem::is_regular_file(flacPath)) {
        loaded = LoadFlac(flacPath, sVillageTrack);
        usedPath = flacPath;
    } else if (!originalFlacPath.empty() && std::filesystem::is_regular_file(originalFlacPath)) {
        loaded = LoadFlac(originalFlacPath, sVillageTrack);
        usedPath = originalFlacPath;
    } else if (!mp3Path.empty() && std::filesystem::is_regular_file(mp3Path)) {
        loaded = LoadMp3(mp3Path, sVillageTrack);
        usedPath = mp3Path;
    } else if (!originalMp3Path.empty() && std::filesystem::is_regular_file(originalMp3Path)) {
        loaded = LoadMp3(originalMp3Path, sVillageTrack);
        usedPath = originalMp3Path;
    } else {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Village file missing; Village area will keep Start music.\n");
        return false;
    }

    if (!loaded) {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Could not decode Village file; Village area will keep Start music.\n");
        return false;
    }

    sVillageReady.store(true, std::memory_order_release);
    std::fprintf(stderr,
                 "[SpaceworldMusic] Loaded Village: %u Hz, %u channels, %.2f seconds (%s).\n",
                 sVillageTrack.sampleRate, sVillageTrack.channels,
                 static_cast<double>(sVillageTrack.frameCount) / sVillageTrack.sampleRate,
                 usedPath.c_str());
    return true;
}

bool LoadTowerTrack() {
    const std::string flacPath =
        Ship::Context::GetPathRelativeToAppDirectory("mods/spaceworld_music/Tower.flac");
    const std::string mp3Path =
        Ship::Context::GetPathRelativeToAppDirectory("mods/spaceworld_music/Tower.mp3");
    const std::string originalFlacPath =
        Ship::Context::GetPathRelativeToAppDirectory(
            "mods/spaceworld_music/17. Mumbo Mountain - Tower (Spaceworld).flac");
    const std::string numberedFlacPath =
        Ship::Context::GetPathRelativeToAppDirectory(
            "mods/spaceworld_music/17. Mumbo Mountain - Tower (Spaceworld)(1).flac");

    bool loaded = false;
    std::string usedPath;

    if (!flacPath.empty() && std::filesystem::is_regular_file(flacPath)) {
        loaded = LoadFlac(flacPath, sTowerTrack);
        usedPath = flacPath;
    } else if (!originalFlacPath.empty() && std::filesystem::is_regular_file(originalFlacPath)) {
        loaded = LoadFlac(originalFlacPath, sTowerTrack);
        usedPath = originalFlacPath;
    } else if (!numberedFlacPath.empty() && std::filesystem::is_regular_file(numberedFlacPath)) {
        loaded = LoadFlac(numberedFlacPath, sTowerTrack);
        usedPath = numberedFlacPath;
    } else if (!mp3Path.empty() && std::filesystem::is_regular_file(mp3Path)) {
        loaded = LoadMp3(mp3Path, sTowerTrack);
        usedPath = mp3Path;
    } else {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Tower file missing; Tower exterior will use Start.\n");
        return false;
    }

    if (!loaded) {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Could not decode Tower file; Tower exterior will use Start.\n");
        return false;
    }

    sTowerReady.store(true, std::memory_order_release);
    std::fprintf(stderr,
                 "[SpaceworldMusic] Loaded Tower: %u Hz, %u channels, %.2f seconds (%s).\n",
                 sTowerTrack.sampleRate, sTowerTrack.channels,
                 static_cast<double>(sTowerTrack.frameCount) / sTowerTrack.sampleRate,
                 usedPath.c_str());
    return true;
}

bool LoadUnderwaterTrack() {
    const std::string flacPath =
        Ship::Context::GetPathRelativeToAppDirectory("mods/spaceworld_music/Underwater.flac");
    const std::string mp3Path =
        Ship::Context::GetPathRelativeToAppDirectory("mods/spaceworld_music/Underwater.mp3");
    const std::string originalFlacPath =
        Ship::Context::GetPathRelativeToAppDirectory(
            "mods/spaceworld_music/15. Mumbo Mountain - Underwater (Spaceworld).flac");
    const std::string numberedFlacPath =
        Ship::Context::GetPathRelativeToAppDirectory(
            "mods/spaceworld_music/15. Mumbo Mountain - Underwater (Spaceworld)(1).flac");

    bool loaded = false;
    std::string usedPath;

    if (!flacPath.empty() && std::filesystem::is_regular_file(flacPath)) {
        loaded = LoadFlac(flacPath, sUnderwaterTrack);
        usedPath = flacPath;
    } else if (!originalFlacPath.empty() &&
               std::filesystem::is_regular_file(originalFlacPath)) {
        loaded = LoadFlac(originalFlacPath, sUnderwaterTrack);
        usedPath = originalFlacPath;
    } else if (!numberedFlacPath.empty() &&
               std::filesystem::is_regular_file(numberedFlacPath)) {
        loaded = LoadFlac(numberedFlacPath, sUnderwaterTrack);
        usedPath = numberedFlacPath;
    } else if (!mp3Path.empty() && std::filesystem::is_regular_file(mp3Path)) {
        loaded = LoadMp3(mp3Path, sUnderwaterTrack);
        usedPath = mp3Path;
    } else {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Underwater file missing; underwater will use Start.\n");
        return false;
    }

    if (!loaded) {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Could not decode Underwater file; underwater will use Start.\n");
        return false;
    }

    sUnderwaterReady.store(true, std::memory_order_release);
    std::fprintf(stderr,
                 "[SpaceworldMusic] Loaded Underwater: %u Hz, %u channels, %.2f seconds (%s).\n",
                 sUnderwaterTrack.sampleRate, sUnderwaterTrack.channels,
                 static_cast<double>(sUnderwaterTrack.frameCount) / sUnderwaterTrack.sampleRate,
                 usedPath.c_str());
    return true;
}


double ReadMonoFrame(const DecodedTrack& track, int64_t frame) {
    if (!track.Ready() || frame < 0 ||
        frame >= static_cast<int64_t>(track.frameCount)) {
        return 0.0;
    }

    const uint64_t base =
        static_cast<uint64_t>(frame) * static_cast<uint64_t>(track.channels);

    if (track.channels == 1) {
        return static_cast<double>(track.samples[static_cast<size_t>(base)]);
    }

    const double left =
        static_cast<double>(track.samples[static_cast<size_t>(base)]);
    const double right =
        static_cast<double>(track.samples[static_cast<size_t>(base + 1)]);
    return (left + right) * 0.5;
}

double ScoreVillageLag(int64_t startCenterFrame, int lagFrames,
                       int windowFrames, int strideFrames) {
    const int64_t halfWindow = windowFrames / 2;
    const int64_t startFirst = startCenterFrame - halfWindow;
    const int64_t villageFirst = startFirst + lagFrames;

    double sumXY = 0.0;
    double sumXX = 0.0;
    double sumYY = 0.0;

    for (int i = 0; i < windowFrames; i += strideFrames) {
        const double x = ReadMonoFrame(sStartTrack, startFirst + i);
        const double y = ReadMonoFrame(sVillageTrack, villageFirst + i);

        sumXY += x * y;
        sumXX += x * x;
        sumYY += y * y;
    }

    if (sumXX <= 1.0 || sumYY <= 1.0) {
        return -1.0;
    }

    return sumXY / std::sqrt(sumXX * sumYY);
}

bool EstimateVillageLagAtTime(double startSeconds, int& lagFramesOut,
                              double& scoreOut) {
    if (!sStartTrack.Ready() || !sVillageTrack.Ready() ||
        sStartTrack.sampleRate != sVillageTrack.sampleRate) {
        return false;
    }

    const int sampleRate = static_cast<int>(sStartTrack.sampleRate);
    const int64_t center =
        static_cast<int64_t>(std::llround(startSeconds * sampleRate));

    const int windowFrames = sampleRate * 2;
    const int halfWindow = windowFrames / 2;
    const int maxLagFrames =
        static_cast<int>(std::llround(sampleRate * 0.100));

    const int64_t minFrames =
        static_cast<int64_t>(std::min(sStartTrack.frameCount,
                                      sVillageTrack.frameCount));

    if (center - halfWindow - maxLagFrames < 0 ||
        center + halfWindow + maxLagFrames >= minFrames) {
        return false;
    }

    int bestLag = 0;
    double bestScore = -2.0;
    constexpr int kCoarseLagStep = 16;
    constexpr int kCoarseSampleStride = 32;

    for (int lag = -maxLagFrames; lag <= maxLagFrames;
         lag += kCoarseLagStep) {
        const double score =
            ScoreVillageLag(center, lag, windowFrames, kCoarseSampleStride);
        if (score > bestScore) {
            bestScore = score;
            bestLag = lag;
        }
    }

    const int fineStart =
        std::max(-maxLagFrames, bestLag - kCoarseLagStep);
    const int fineEnd =
        std::min(maxLagFrames, bestLag + kCoarseLagStep);
    constexpr int kFineSampleStride = 8;

    for (int lag = fineStart; lag <= fineEnd; lag++) {
        const double score =
            ScoreVillageLag(center, lag, windowFrames, kFineSampleStride);
        if (score > bestScore) {
            bestScore = score;
            bestLag = lag;
        }
    }

    lagFramesOut = bestLag;
    scoreOut = bestScore;
    return true;
}

void CalibrateVillageSync() {
    sVillageSyncScale = 1.0;
    sVillageSyncOffsetSeconds = 0.0;
    sVillageSyncCalibrated = false;

    if (!sStartTrack.Ready() || !sVillageTrack.Ready()) {
        return;
    }

    if (sStartTrack.sampleRate != sVillageTrack.sampleRate) {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Auto-sync skipped: sample rates differ.\n");
        return;
    }

    struct SyncPoint {
        double time;
        double lagSeconds;
        double score;
    };

    std::vector<SyncPoint> points;
    constexpr double kAnchorTimes[] = {
        20.0, 35.0, 50.0, 65.0, 80.0, 95.0, 110.0, 120.0
    };

    const double sampleRate = static_cast<double>(sStartTrack.sampleRate);

    for (double time : kAnchorTimes) {
        int lagFrames = 0;
        double score = 0.0;

        if (!EstimateVillageLagAtTime(time, lagFrames, score)) {
            continue;
        }

        if (score < 0.12) {
            continue;
        }

        const double lagSeconds =
            static_cast<double>(lagFrames) / sampleRate;

        points.push_back({ time, lagSeconds, score });
    }

    if (points.empty()) {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Auto-sync found no reliable shared audio.\n");
        return;
    }

    std::vector<double> lags;
    lags.reserve(points.size());
    for (const auto& point : points) {
        lags.push_back(point.lagSeconds);
    }
    std::sort(lags.begin(), lags.end());
    const double medianLag = lags[lags.size() / 2];

    std::vector<SyncPoint> good;
    for (const auto& point : points) {
        if (std::fabs(point.lagSeconds - medianLag) <= 0.005) {
            good.push_back(point);
        }
    }

    if (good.empty()) {
        return;
    }

    if (good.size() == 1) {
        sVillageSyncOffsetSeconds = good[0].lagSeconds;
        sVillageSyncScale = 1.0;
        sVillageSyncCalibrated = true;
    } else {
        double sw = 0.0;
        double st = 0.0;
        double sl = 0.0;
        double stt = 0.0;
        double stl = 0.0;

        for (const auto& point : good) {
            const double w = std::max(0.01, point.score * point.score);
            sw += w;
            st += w * point.time;
            sl += w * point.lagSeconds;
            stt += w * point.time * point.time;
            stl += w * point.time * point.lagSeconds;
        }

        const double denom = sw * stt - st * st;
        double slope = 0.0;
        double intercept = medianLag;

        if (std::fabs(denom) > 1e-12) {
            slope = (sw * stl - st * sl) / denom;
            intercept = (sl - slope * st) / sw;
        }

        if (std::fabs(slope) > 0.0001) {
            slope = 0.0;
            intercept = medianLag;
        }

        sVillageSyncScale = 1.0 + slope;
        sVillageSyncOffsetSeconds = intercept;
        sVillageSyncCalibrated = true;
    }
}

double MapStartTimeToVillage(double startSeconds) {
    return startSeconds * sVillageSyncScale + sVillageSyncOffsetSeconds;
}

double ScoreTowerLag(int64_t startCenterFrame, int lagFrames,
                     int windowFrames, int strideFrames) {
    const int64_t halfWindow = windowFrames / 2;
    const int64_t startFirst = startCenterFrame - halfWindow;
    const int64_t towerFirst = startFirst + lagFrames;

    double sumXY = 0.0;
    double sumXX = 0.0;
    double sumYY = 0.0;

    for (int i = 0; i < windowFrames; i += strideFrames) {
        const double x = ReadMonoFrame(sStartTrack, startFirst + i);
        const double y = ReadMonoFrame(sTowerTrack, towerFirst + i);

        sumXY += x * y;
        sumXX += x * x;
        sumYY += y * y;
    }

    if (sumXX <= 1.0 || sumYY <= 1.0) {
        return -1.0;
    }

    return sumXY / std::sqrt(sumXX * sumYY);
}

bool EstimateTowerLagAtTime(double startSeconds, int& lagFramesOut,
                            double& scoreOut) {
    if (!sStartTrack.Ready() || !sTowerTrack.Ready() ||
        sStartTrack.sampleRate != sTowerTrack.sampleRate) {
        return false;
    }

    const int sampleRate = static_cast<int>(sStartTrack.sampleRate);
    const int64_t center =
        static_cast<int64_t>(std::llround(startSeconds * sampleRate));

    const int windowFrames = sampleRate * 2;
    const int halfWindow = windowFrames / 2;

    // Search a broad range from the exact decoded recordings instead of
    // assuming an externally measured Tower timestamp.
    const int maxLagFrames =
        static_cast<int>(std::llround(sampleRate * 0.250));

    const int64_t minFrames =
        static_cast<int64_t>(std::min(sStartTrack.frameCount,
                                      sTowerTrack.frameCount));

    if (center - halfWindow - maxLagFrames < 0 ||
        center + halfWindow + maxLagFrames >= minFrames) {
        return false;
    }

    int bestLag = 0;
    double bestScore = -2.0;
    constexpr int kCoarseLagStep = 16;
    constexpr int kCoarseSampleStride = 32;

    for (int lag = -maxLagFrames; lag <= maxLagFrames;
         lag += kCoarseLagStep) {
        const double score =
            ScoreTowerLag(center, lag, windowFrames, kCoarseSampleStride);
        if (score > bestScore) {
            bestScore = score;
            bestLag = lag;
        }
    }

    const int fineStart =
        std::max(-maxLagFrames, bestLag - kCoarseLagStep);
    const int fineEnd =
        std::min(maxLagFrames, bestLag + kCoarseLagStep);
    constexpr int kFineSampleStride = 8;

    for (int lag = fineStart; lag <= fineEnd; lag++) {
        const double score =
            ScoreTowerLag(center, lag, windowFrames, kFineSampleStride);
        if (score > bestScore) {
            bestScore = score;
            bestLag = lag;
        }
    }

    lagFramesOut = bestLag;
    scoreOut = bestScore;
    return true;
}

void CalibrateTowerSync() {
    sTowerSyncScale = 1.0;
    sTowerSyncOffsetSeconds = 0.0;
    sTowerSyncCalibrated = false;

    if (!sStartTrack.Ready() || !sTowerTrack.Ready()) {
        return;
    }

    if (sStartTrack.sampleRate != sTowerTrack.sampleRate) {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Tower auto-sync skipped: sample rates differ.\n");
        return;
    }

    struct SyncPoint {
        double time;
        double lagSeconds;
        double score;
    };

    std::vector<SyncPoint> points;
    constexpr double kAnchorTimes[] = {
        20.0, 35.0, 50.0, 65.0, 80.0, 95.0, 110.0, 120.0
    };

    const double sampleRate =
        static_cast<double>(sStartTrack.sampleRate);

    for (double time : kAnchorTimes) {
        int lagFrames = 0;
        double score = 0.0;

        if (!EstimateTowerLagAtTime(time, lagFrames, score)) {
            continue;
        }

        if (score < 0.70) {
            continue;
        }

        const double lagSeconds =
            static_cast<double>(lagFrames) / sampleRate;

        points.push_back({ time, lagSeconds, score });
    }

    if (points.size() < 4) {
        std::fprintf(stderr,
                     "[SpaceworldMusic] Tower auto-sync found fewer than four reliable anchors.\n");
        return;
    }

    std::vector<double> lags;
    lags.reserve(points.size());
    for (const auto& point : points) {
        lags.push_back(point.lagSeconds);
    }
    std::sort(lags.begin(), lags.end());
    const double medianLag = lags[lags.size() / 2];

    std::vector<SyncPoint> good;
    for (const auto& point : points) {
        if (std::fabs(point.lagSeconds - medianLag) <= 0.005) {
            good.push_back(point);
        }
    }

    if (good.empty()) {
        return;
    }

    if (good.size() == 1) {
        sTowerSyncOffsetSeconds = good[0].lagSeconds;
        sTowerSyncScale = 1.0;
        sTowerSyncCalibrated = true;
    } else {
        double sw = 0.0;
        double st = 0.0;
        double sl = 0.0;
        double stt = 0.0;
        double stl = 0.0;

        for (const auto& point : good) {
            const double w = std::max(0.01, point.score * point.score);
            sw += w;
            st += w * point.time;
            sl += w * point.lagSeconds;
            stt += w * point.time * point.time;
            stl += w * point.time * point.lagSeconds;
        }

        const double denom = sw * stt - st * st;
        double slope = 0.0;
        double intercept = medianLag;

        if (std::fabs(denom) > 1e-12) {
            slope = (sw * stl - st * sl) / denom;
            intercept = (sl - slope * st) / sw;
        }

        if (std::fabs(slope) > 0.0001) {
            slope = 0.0;
            intercept = medianLag;
        }

        sTowerSyncScale = 1.0 + slope;
        sTowerSyncOffsetSeconds = intercept;
        sTowerSyncCalibrated = true;
    }
}

double MapStartTimeToTower(double startSeconds) {
    return startSeconds * sTowerSyncScale + sTowerSyncOffsetSeconds;
}

void CalibrateUnderwaterSyncAndHardLoop() {
    sUnderwaterSyncScale = 1.0;
    sUnderwaterSyncOffsetSeconds = 0.0;
    sUnderwaterSyncCalibrated = false;

    if (!sStartTrack.Ready() || !sUnderwaterTrack.Ready() ||
        sLoopLengthSeconds <= 0.0) {
        return;
    }

    // Measured directly from the supplied Start + Underwater recordings.
    // Underwater runs about 278 ppm faster. These values only center the
    // search; the final mapping comes from the exact decoded Underwater PCM.
    constexpr double kMeasuredScale = 0.9997218764310626;
    constexpr double kMeasuredOffsetSeconds = 0.0157458484955960;
    constexpr int64_t kSearchRadiusFrames = 220; // +/-10 ms on the 22 kHz mixer grid.

    const double predictedStartSeconds =
        sLoopStartSeconds * kMeasuredScale + kMeasuredOffsetSeconds;
    const double predictedEndSeconds =
        sLoopEndSeconds * kMeasuredScale + kMeasuredOffsetSeconds;

    const int64_t centerStart =
        static_cast<int64_t>(std::llround(
            predictedStartSeconds * kOutputSampleRate));
    const int64_t centerEnd =
        static_cast<int64_t>(std::llround(
            predictedEndSeconds * kOutputSampleRate));

    double bestScore = 1.0e300;
    int64_t bestStart = centerStart;
    int64_t bestEnd = centerEnd;

    for (int64_t startFrame = centerStart - kSearchRadiusFrames;
         startFrame <= centerStart + kSearchRadiusFrames;
         startFrame++) {
        for (int64_t endFrame = centerEnd - kSearchRadiusFrames;
             endFrame <= centerEnd + kSearchRadiusFrames;
             endFrame++) {
            if (endFrame <= startFrame + 1) {
                continue;
            }

            const double score =
                ScoreHardLoopCandidate(
                    sUnderwaterTrack, startFrame, endFrame);

            if (score < bestScore) {
                bestScore = score;
                bestStart = startFrame;
                bestEnd = endFrame;
            }
        }
    }

    const double underwaterLoopStart =
        static_cast<double>(bestStart) / kOutputSampleRate;
    const double underwaterLoopEnd =
        static_cast<double>(bestEnd) / kOutputSampleRate;
    const double underwaterLoopLength =
        underwaterLoopEnd - underwaterLoopStart;

    if (underwaterLoopLength <= 0.0) {
        return;
    }

    // Map Start's exact decoded loop endpoints to Underwater's own
    // clean hard-loop endpoints. This keeps the two recordings in phase
    // while also avoiding a click at the shared master wrap.
    sUnderwaterSyncScale =
        underwaterLoopLength / sLoopLengthSeconds;
    sUnderwaterSyncOffsetSeconds =
        underwaterLoopStart -
        sLoopStartSeconds * sUnderwaterSyncScale;
    sUnderwaterSyncCalibrated = true;
}

double MapStartTimeToUnderwater(double startSeconds) {
    return startSeconds * sUnderwaterSyncScale +
           sUnderwaterSyncOffsetSeconds;
}

void LoadTracks() {
    LoadStartTrack();
    LoadVillageTrack();
    LoadTowerTrack();
    LoadUnderwaterTrack();
    CalibrateVillageSync();
    CalibrateTowerSync();
    CalibrateUnderwaterSyncAndHardLoop();
    sResetPlayback.store(true, std::memory_order_release);
}

double WrapTimeline(double seconds) {
    if (seconds < sLoopEndSeconds) {
        return seconds;
    }

    if (sLoopLengthSeconds <= 0.0) {
        return sLoopStartSeconds;
    }

    return sLoopStartSeconds +
           std::fmod(seconds - sLoopStartSeconds, sLoopLengthSeconds);
}

int16_t ReadTrackSample(const DecodedTrack& track, double seconds, uint32_t channel) {
    if (!track.Ready()) {
        return 0;
    }

    const double wrappedSeconds = WrapTimeline(seconds);
    double framePosition = wrappedSeconds * static_cast<double>(track.sampleRate);

    if (framePosition < 0.0) {
        framePosition = 0.0;
    }

    const double maxFrame = static_cast<double>(track.frameCount - 1);
    if (framePosition > maxFrame) {
        framePosition = maxFrame;
    }

    const uint64_t frame0 = static_cast<uint64_t>(framePosition);
    const uint64_t frame1 =
        (frame0 + 1 < track.frameCount) ? frame0 + 1 : frame0;
    const double fraction = framePosition - static_cast<double>(frame0);

    uint32_t actualChannel = channel;
    if (track.channels == 1) {
        actualChannel = 0;
    } else if (actualChannel >= track.channels) {
        actualChannel = track.channels - 1;
    }

    const uint64_t index0 = frame0 * track.channels + actualChannel;
    const uint64_t index1 = frame1 * track.channels + actualChannel;

    const double a = track.samples[static_cast<size_t>(index0)];
    const double b = track.samples[static_cast<size_t>(index1)];
    return static_cast<int16_t>(a + (b - a) * fraction);
}

int16_t ReadTrackSampleUnwrapped(const DecodedTrack& track, double seconds,
                                 uint32_t channel) {
    if (!track.Ready()) {
        return 0;
    }

    double framePosition =
        seconds * static_cast<double>(track.sampleRate);

    if (framePosition < 0.0) {
        framePosition = 0.0;
    }

    const double maxFrame =
        static_cast<double>(track.frameCount - 1);
    if (framePosition > maxFrame) {
        framePosition = maxFrame;
    }

    const uint64_t frame0 =
        static_cast<uint64_t>(framePosition);
    const uint64_t frame1 =
        (frame0 + 1 < track.frameCount) ? frame0 + 1 : frame0;
    const double fraction =
        framePosition - static_cast<double>(frame0);

    uint32_t actualChannel = channel;
    if (track.channels == 1) {
        actualChannel = 0;
    } else if (actualChannel >= track.channels) {
        actualChannel = track.channels - 1;
    }

    const uint64_t index0 =
        frame0 * track.channels + actualChannel;
    const uint64_t index1 =
        frame1 * track.channels + actualChannel;

    const double a =
        track.samples[static_cast<size_t>(index0)];
    const double b =
        track.samples[static_cast<size_t>(index1)];

    return static_cast<int16_t>(a + (b - a) * fraction);
}


int GetPrimaryLogicalVolume() {
    if (comusicTracks == nullptr || comusicTracks[0].track_id != COMUSIC_2_MM) {
        return 0;
    }
    return std::max(0, comusicTracks[0].volume);
}

float GetPrimaryLogicalGain() {
    const int defaultVolume = gcMusic_getDefaultVolumeForTrack(COMUSIC_2_MM);
    if (defaultVolume <= 0) {
        return 0.0f;
    }
    const float gain =
        static_cast<float>(GetPrimaryLogicalVolume()) / static_cast<float>(defaultVolume);
    return std::clamp(gain, 0.0f, 1.0f);
}


void RestoreRetailMMPhysicalVolume() {
    port_spaceworldMusicRefreshPhysicalMusicSlotVolume(0);
}



void UpdateSpaceworldMusic() {
    const bool enabled = CVarGetInteger(CVAR_SPACEWORLD_MUSIC, 0) != 0;
    const enum map_e currentMap = gsworld_getMap();
    const bool isMM = currentMap == MAP_2_MM_MUMBOS_MOUNTAIN;

    // MM's Witch Switch deliberately carries the current Mumbo's Mountain
    // music into MAP_69_GL_MM_LOBBY for the jiggy-reveal cutscene via
    // musicKeepsPlaying(). Keep the Spaceworld mixer owning COMUSIC_2_MM
    // during that carried-music window so the physical retail sequence
    // cannot leak back in when the map changes.
    const bool mmWitchSwitchMusicCarry =
        currentMap == MAP_69_GL_MM_LOBBY &&
        enabled &&
        sStartReady.load(std::memory_order_acquire) &&
        sMixActive.load(std::memory_order_acquire) &&
        musicSlot_getTrack(0) == COMUSIC_2_MM;

    if (mmWitchSwitchMusicCarry) {
        sPrimaryMusicGain.store(GetPrimaryLogicalGain(), std::memory_order_release);
        sMixActive.store(true, std::memory_order_release);
        port_spaceworldMusicRefreshPhysicalMusicSlotVolume(0);
        return;
    }

    if (!isMM) {
        if (sMixActive.exchange(false, std::memory_order_acq_rel)) {
            RestoreRetailMMPhysicalVolume();
        }
        sPrimaryMusicGain.store(0.0f, std::memory_order_release);
        sResetPlayback.store(true, std::memory_order_release);
        sDesiredVariant.store(SPACEWORLD_START, std::memory_order_release);
        sLoadAttempted = false;
        return;
    }

    // Keep the old special-mode exclusion, but allow the MM attract demo
    // to use the Spaceworld restoration. This changes music only; it does
    // not touch demo gameplay, culling, RNG, or playback state.
    if (func_802E4A08() && getGameMode() != GAME_MODE_7_ATTRACT_DEMO) {
        if (sMixActive.exchange(false, std::memory_order_acq_rel)) {
            RestoreRetailMMPhysicalVolume();
        }
        sPrimaryMusicGain.store(0.0f, std::memory_order_release);
        return;
    }

    if (!enabled) {
        if (sMixActive.exchange(false, std::memory_order_acq_rel)) {
            RestoreRetailMMPhysicalVolume();
        }
        sPrimaryMusicGain.store(0.0f, std::memory_order_release);
        sResetPlayback.store(true, std::memory_order_release);
        sDesiredVariant.store(SPACEWORLD_START, std::memory_order_release);
        sLoadAttempted = false;
        return;
    }

    if (!sStartReady.load(std::memory_order_acquire) && !sLoadAttempted) {
        sLoadAttempted = true;
        LoadTracks();
    }

    if (!sStartReady.load(std::memory_order_acquire)) {
        if (sMixActive.exchange(false, std::memory_order_acq_rel)) {
            RestoreRetailMMPhysicalVolume();
        }
        sPrimaryMusicGain.store(0.0f, std::memory_order_release);
        return;
    }

    // Keep the beta timeline alive silently when Banjo temporarily suppresses
    // or replaces the primary MM track. This prevents fanfares / Witch Switch
    // events from restarting the recording.
    if (musicSlot_getTrack(0) != COMUSIC_2_MM) {
        sPrimaryMusicGain.store(0.0f, std::memory_order_release);
        sMixActive.store(true, std::memory_order_release);
        return;
    }
    // Retail's channel-mask controller now decides which recording variant
    // is active. Do not duplicate its area/position rules here.
    sDesiredVariant.store(
        VariantFromRetailChannelMask(
            sRetailPrimaryChannelMask.load(std::memory_order_acquire)),
        std::memory_order_release);

    // Banjo's logical primary music volume is the master gain for the beta
    // recording. Normal fades/stops therefore apply automatically.
    sPrimaryMusicGain.store(GetPrimaryLogicalGain(), std::memory_order_release);
    // Turn on external ownership, then silence only the retail sequence's
    // PHYSICAL output. Its volume, play/stop state, sequence position and
    // channel-mask machinery continue running exactly as stock.
    sMixActive.store(true, std::memory_order_release);
    port_spaceworldMusicRefreshPhysicalMusicSlotVolume(0);
}

void RegisterSpaceworldMusic_Init() {
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_NORMAL,
                      [](IEvent*) { UpdateSpaceworldMusic(); });
}

static RegisterShipInitFunc sInit(RegisterSpaceworldMusic_Init, { CVAR_SPACEWORLD_MUSIC });

} // namespace


extern "C" void port_spaceworldMusicOnMusicSlotVolume(int32_t index, int32_t trackId, int32_t volume) {
    if (index != 0 ||
        sIgnorePrimaryMusicHook.load(std::memory_order_acquire) ||
        !sMixActive.load(std::memory_order_acquire)) {
        return;
    }

    if (trackId != COMUSIC_2_MM) {
        sPrimaryMusicGain.store(0.0f, std::memory_order_release);
        return;
    }

    const int defaultVolume = gcMusic_getDefaultVolumeForTrack(COMUSIC_2_MM);
    const float gain = defaultVolume > 0
        ? static_cast<float>(std::max(0, volume)) / static_cast<float>(defaultVolume)
        : 0.0f;
    sPrimaryMusicGain.store(std::clamp(gain, 0.0f, 1.0f), std::memory_order_release);
}


extern "C" void port_spaceworldMusicOnMusicSlotTrack(int32_t index, int32_t trackId) {
    if (index != 0) {
        return;
    }

    const int32_t previousTrack =
        sRetailPrimaryTrack.exchange(trackId, std::memory_order_acq_rel);

    if (trackId == COMUSIC_2_MM) {
        // A genuine reload of retail MM starts its sequence from the beginning.
        // Mirror that lifecycle in the external recording.
        if (previousTrack != COMUSIC_2_MM &&
            sMixActive.load(std::memory_order_acquire)) {
            sResetPlayback.store(true, std::memory_order_release);
        }

        sPrimaryMusicGain.store(GetPrimaryLogicalGain(), std::memory_order_release);
    } else {
        sRetailPrimaryPlaying.store(false, std::memory_order_release);
        sPrimaryMusicGain.store(0.0f, std::memory_order_release);
    }
}



extern "C" int32_t port_spaceworldMusicGetPhysicalMusicSlotVolume(
    int32_t index, int32_t trackId, int32_t requestedVolume) {
    // The logical/requested MM volume must remain untouched because the
    // existing exact hook uses it to fade the Spaceworld recording.
    //
    // Only the physical retail sequence output is suppressed while the
    // Spaceworld mixer owns MM. This prevents pause-volume updates from
    // briefly exposing the retail song between game-frame mute calls.
    if (index == 0 &&
        trackId == COMUSIC_2_MM &&
        sMixActive.load(std::memory_order_acquire)) {
        return 0;
    }

    return requestedVolume;
}


extern "C" void port_spaceworldMusicOnMusicSlotTransport(
    int32_t index, int32_t trackId, int32_t playing) {
    if (index != 0) {
        return;
    }

    if (trackId == COMUSIC_2_MM) {
        sRetailPrimaryPlaying.store(playing != 0, std::memory_order_release);
    } else if (playing == 0) {
        sRetailPrimaryPlaying.store(false, std::memory_order_release);
    }
}

extern "C" void port_spaceworldMusicOnMusicSlotChannelMask(
    int32_t index, int32_t trackId, int32_t channelMask, float transitionSpeed) {
    if (index != 0) {
        return;
    }

    sRetailPrimaryChannelMask.store(channelMask, std::memory_order_release);
    sDesiredVariant.store(
        VariantFromRetailChannelMask(channelMask),
        std::memory_order_release);

    // Banjo's channel transition code reaches the target over
    // transitionSpeed * FRAMERATE / 2 updates, i.e. transitionSpeed / 2
    // seconds at the original update rate.
    const float seconds = std::max(0.0f, transitionSpeed * 0.5f);
    sRetailVariantTransitionSeconds.store(seconds, std::memory_order_release);
}

extern "C" void port_spaceworldMusicMix(int16_t* samples, size_t sampleCount) {
    if (samples == nullptr || sampleCount < 2 ||
        !sMixActive.load(std::memory_order_acquire) ||
        !sStartReady.load(std::memory_order_acquire) ||
        !sStartTrack.Ready()) {
        return;
    }

    if (sResetPlayback.exchange(false, std::memory_order_acq_rel)) {
        sTimelineSeconds = 0.0;

        // Initialize directly from retail's already-selected arrangement.
        const int initialVariant =
            sDesiredVariant.load(std::memory_order_acquire);
        sVillageBlend =
            initialVariant == SPACEWORLD_VILLAGE ? 1.0 : 0.0;
        sTowerBlend =
            initialVariant == SPACEWORLD_TOWER &&
            sTowerReady.load(std::memory_order_acquire) &&
            sTowerSyncCalibrated
                ? 1.0
                : 0.0;
        sUnderwaterBlend =
            initialVariant == SPACEWORLD_UNDERWATER &&
            sUnderwaterReady.load(std::memory_order_acquire) &&
            sUnderwaterSyncCalibrated
                ? 1.0
                : 0.0;
    }

    const int desiredVariant =
        sDesiredVariant.load(std::memory_order_acquire);

    const bool wantVillage =
        desiredVariant == SPACEWORLD_VILLAGE &&
        sVillageReady.load(std::memory_order_acquire) &&
        sVillageTrack.Ready();

    const bool wantTower =
        desiredVariant == SPACEWORLD_TOWER &&
        sTowerReady.load(std::memory_order_acquire) &&
        sTowerTrack.Ready() &&
        sTowerSyncCalibrated;

    const bool wantUnderwater =
        desiredVariant == SPACEWORLD_UNDERWATER &&
        sUnderwaterReady.load(std::memory_order_acquire) &&
        sUnderwaterTrack.Ready() &&
        sUnderwaterSyncCalibrated;

    const double villageTarget = wantVillage ? 1.0 : 0.0;
    const double towerTarget = wantTower ? 1.0 : 0.0;
    const double underwaterTarget = wantUnderwater ? 1.0 : 0.0;

    const double retailTransitionSeconds = static_cast<double>(
        sRetailVariantTransitionSeconds.load(std::memory_order_acquire));
    const double blendStep = retailTransitionSeconds > 0.0
        ? 1.0 / (retailTransitionSeconds * kOutputSampleRate)
        : 1.0;

    const bool retailTransportPlaying =
        sRetailPrimaryTrack.load(std::memory_order_acquire) == COMUSIC_2_MM &&
        sRetailPrimaryPlaying.load(std::memory_order_acquire);

    const size_t outputFrames = sampleCount / 2;

    for (size_t frame = 0; frame < outputFrames; frame++) {
        if (retailTransportPlaying) {
            if (sVillageBlend < villageTarget) {
                sVillageBlend =
                    std::min(villageTarget, sVillageBlend + blendStep);
            } else if (sVillageBlend > villageTarget) {
                sVillageBlend =
                    std::max(villageTarget, sVillageBlend - blendStep);
            }

            if (sTowerBlend < towerTarget) {
                sTowerBlend =
                    std::min(towerTarget, sTowerBlend + blendStep);
            } else if (sTowerBlend > towerTarget) {
                sTowerBlend =
                    std::max(towerTarget, sTowerBlend - blendStep);
            }

            if (sUnderwaterBlend < underwaterTarget) {
                sUnderwaterBlend =
                    std::min(underwaterTarget, sUnderwaterBlend + blendStep);
            } else if (sUnderwaterBlend > underwaterTarget) {
                sUnderwaterBlend =
                    std::max(underwaterTarget, sUnderwaterBlend - blendStep);
            }
        }

        double villageWeight =
            std::clamp(sVillageBlend, 0.0, 1.0);
        double towerWeight =
            std::clamp(sTowerBlend, 0.0, 1.0);
        double underwaterWeight =
            std::clamp(sUnderwaterBlend, 0.0, 1.0);
        double startWeight =
            std::max(
                0.0,
                1.0 - villageWeight - towerWeight - underwaterWeight);

        const double weightSum =
            startWeight + villageWeight + towerWeight + underwaterWeight;
        if (weightSum > 0.0) {
            startWeight /= weightSum;
            villageWeight /= weightSum;
            towerWeight /= weightSum;
            underwaterWeight /= weightSum;
        }

        const int32_t startLeft =
            ReadTrackSample(sStartTrack, sTimelineSeconds, 0);
        const int32_t startRight =
            ReadTrackSample(sStartTrack, sTimelineSeconds, 1);

        int32_t villageLeft = 0;
        int32_t villageRight = 0;

        if (sVillageTrack.Ready()) {
            const double villageSeconds =
                MapStartTimeToVillage(sTimelineSeconds);

            villageLeft =
                ReadTrackSampleUnwrapped(
                    sVillageTrack, villageSeconds, 0);
            villageRight =
                ReadTrackSampleUnwrapped(
                    sVillageTrack, villageSeconds, 1);
        }

        int32_t towerLeft = 0;
        int32_t towerRight = 0;

        if (sTowerTrack.Ready() && sTowerSyncCalibrated) {
            const double towerSeconds =
                MapStartTimeToTower(sTimelineSeconds);

            towerLeft =
                ReadTrackSampleUnwrapped(
                    sTowerTrack, towerSeconds, 0);
            towerRight =
                ReadTrackSampleUnwrapped(
                    sTowerTrack, towerSeconds, 1);
        }

        int32_t underwaterLeft = 0;
        int32_t underwaterRight = 0;

        if (sUnderwaterTrack.Ready() && sUnderwaterSyncCalibrated) {
            const double underwaterSeconds =
                MapStartTimeToUnderwater(sTimelineSeconds);

            underwaterLeft =
                ReadTrackSampleUnwrapped(
                    sUnderwaterTrack, underwaterSeconds, 0);
            underwaterRight =
                ReadTrackSampleUnwrapped(
                    sUnderwaterTrack, underwaterSeconds, 1);
        }

        int32_t customLeft = static_cast<int32_t>(
            startLeft * startWeight +
            villageLeft * villageWeight +
            towerLeft * towerWeight +
            underwaterLeft * underwaterWeight);
        int32_t customRight = static_cast<int32_t>(
            startRight * startWeight +
            villageRight * villageWeight +
            towerRight * towerWeight +
            underwaterRight * underwaterWeight);

        const float primaryMusicGain = retailTransportPlaying
            ? sPrimaryMusicGain.load(std::memory_order_acquire)
            : 0.0f;

        customLeft = static_cast<int32_t>(
            customLeft * kMusicGain * primaryMusicGain);
        customRight = static_cast<int32_t>(
            customRight * kMusicGain * primaryMusicGain);

        const size_t leftIndex = frame * 2;
        const size_t rightIndex = leftIndex + 1;

        samples[leftIndex] = static_cast<int16_t>(std::clamp(
            static_cast<int32_t>(samples[leftIndex]) + customLeft,
            -32768, 32767));
        samples[rightIndex] = static_cast<int16_t>(std::clamp(
            static_cast<int32_t>(samples[rightIndex]) + customRight,
            -32768, 32767));

        if (retailTransportPlaying) {
            sTimelineSeconds += 1.0 / kOutputSampleRate;
            sTimelineSeconds = WrapTimeline(sTimelineSeconds);
        }
    }
}
