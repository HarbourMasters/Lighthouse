#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Retention/Retention.h"
#include "HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "actor.h"
#include "core2/abilityprogress.h"

extern ActorArray* suBaddieActorArray;

typedef struct {
    u8 uid;
    u8 state;
    u8 next_state;
    f32 duration;
    s32 model_index;
    s32 anim_index;
    f32 scale;
} TransitionInfoEntry;
extern TransitionInfoEntry D_8036C150[0x16];

extern s32 print_sCurrentFont;
extern s32 print_sMonospacedModeEnabled;
extern char print_sPreviousBoldLetter;
f32 print_calculateLetterXPos(u8 letter, f32* xPtr, f32* yPtr, f32 arg3);
}

namespace {

void ApplyNoteSignHooks();
f32 MeasureBoldNameWidth(const char* s);
void ApplyPauseNameCentering();
void ApplyDialogSuppression();
void ApplyForceAbilitiesUsed();

int sNoteSignActorId = -1;
bool sSuppressBottlesExplainer = false;
const int* sSuppressedDialogs = nullptr;
int sSuppressedDialogCount = 0;
s32 sForcedUsedAbilities = 0;

// Romhacks that have note signs and Bottles explainers don't need them when
// note saving is turned on. Suppress them.
void ApplyNoteSignHooks() {
    const bool noteRetention = CVarGetInteger(CVAR_NOTE_RETENTION, 0);
    const bool active = sNoteSignActorId >= 0 && noteRetention;
    const bool suppressBottles = sSuppressBottlesExplainer && noteRetention;

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, active, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnActorSpawn*>(event);
        if (ev->actorId == sNoteSignActorId) {
            ev->result = nullptr;
            event->Cancelled = true;
        }
    });

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, suppressBottles, [](IEvent*) {
        if (suBaddieActorArray == nullptr) {
            return;
        }
        Actor* start = suBaddieActorArray->data;
        Actor* end = start + suBaddieActorArray->cnt;
        for (Actor* actor = start; actor < end; actor++) {
            if (actor->marker == nullptr || actor->marker->id != MARKER_B7_TUTORIAL_BOTTLES ||
                actor->actorTypeSpecificField != 8) {
                continue;
            }
            if (actor->partnerActor != nullptr) {
                marker_despawn(actor->partnerActor);
            }
            marker_despawn(actor->marker);
        }
    });
}

// Center bold font in pause menu
f32 MeasureBoldNameWidth(const char* s) {
    const s32 savedSlot = print_sCurrentFont;
    const s32 savedMono = print_sMonospacedModeEnabled;
    const char savedPrev = print_sPreviousBoldLetter;
    print_sCurrentFont = 1;
    print_sMonospacedModeEnabled = 0;
    print_sPreviousBoldLetter = 0;
    f32 x = 0.0f;
    f32 y = 0.0f;
    for (const char* p = s; *p != '\0'; ++p) {
        print_calculateLetterXPos((u8)(unsigned char)*p, &x, &y, 1.05f);
    }
    print_sCurrentFont = savedSlot;
    print_sMonospacedModeEnabled = savedMono;
    print_sPreviousBoldLetter = savedPrev;
    return x;
}

void ApplyPauseNameCentering() {
    COND_VB_SHOULD(VB_PAUSEMENU_LEVEL_NAME_X, EVENT_PRIORITY_NORMAL, port_getRomhackIdentifier() != nullptr, {
        s32* x = va_arg(args, s32*);
        va_arg(args, s32);
        const char* vanillaName = va_arg(args, const char*);
        const char* romhackName = va_arg(args, const char*);
        if (romhackName != nullptr && vanillaName != nullptr) {
            const f32 shift = (MeasureBoldNameWidth(vanillaName) - MeasureBoldNameWidth(romhackName)) * 0.5f;
            *x += (s32)(shift >= 0.0f ? shift + 0.5f : shift - 0.5f);
        }
        (void)should;
    });
}

// Suppress a caller-supplied set of dialogs
void ApplyDialogSuppression() {
    COND_VB_SHOULD(VB_OVERRIDE_DIALOG_SHOW, EVENT_PRIORITY_NORMAL, sSuppressedDialogCount > 0, {
        const s32 textId = va_arg(args, s32);
        for (int i = 0; i < sSuppressedDialogCount; i++) {
            if (sSuppressedDialogs[i] == textId) {
                *should = true;
                break;
            }
        }
    });
}

// Music keeps playing across warps whose endpoints share a group
const WarpMusicGroup* sWarpMusicGroups = nullptr;
int sWarpMusicGroupCount = 0;

bool WarpInMusicGroup(const WarpMusicGroup& group, s32 a, s32 b) {
    bool hasA = false, hasB = false;
    for (int i = 0; i < group.count; i++) {
        hasA = hasA || group.maps[i] == a;
        hasB = hasB || group.maps[i] == b;
    }
    return hasA && hasB;
}

void ApplyWarpMusicGroups() {
    COND_VB_SHOULD(VB_WARP_KEEPS_MUSIC, EVENT_PRIORITY_NORMAL, sWarpMusicGroupCount > 0, {
        const s32 dest = va_arg(args, s32);
        const s32 cur = gsworld_getMap();
        for (int i = 0; i < sWarpMusicGroupCount; i++) {
            if (WarpInMusicGroup(sWarpMusicGroups[i], cur, dest)) {
                musicKeepsPlaying();
                break;
            }
        }
        (void)should;
    });
}

// Mark moves as already used
void ApplyForceAbilitiesUsed() {
    COND_HOOK(OnSaveLoad, EVENT_PRIORITY_NORMAL, sForcedUsedAbilities != 0, [](IEvent*) {
        for (int move = 0; move < 32; move++) {
            if (sForcedUsedAbilities & (1 << move)) {
                ability_setUsed(static_cast<ability_used_e>(move));
            }
        }
    });
}

// Relocated jiggies: vanilla ids retallied under the level that now hosts them
const JiggyRelocation* sJiggyRelocations = nullptr;
int sJiggyRelocationCount = 0;
const int* sJiggyExcluded = nullptr;
int sJiggyExcludedCount = 0;

bool JiggyRelocated(s32 id) {
    for (int g = 0; g < sJiggyRelocationCount; g++) {
        for (int i = 0; i < sJiggyRelocations[g].count; i++) {
            if (sJiggyRelocations[g].ids[i] == id) {
                return true;
            }
        }
    }
    for (int i = 0; i < sJiggyExcludedCount; i++) {
        if (sJiggyExcluded[i] == id) {
            return true;
        }
    }
    return false;
}

void ApplyJiggyRelocation() {
    COND_VB_SHOULD(VB_JIGGYSCORE_LEVEL_TOTAL, EVENT_PRIORITY_NORMAL, sJiggyRelocationCount > 0, {
        const s32 level = va_arg(args, s32);
        s32* result = va_arg(args, s32*);
        s32 total = 0;

        // Vanilla groups jiggies into ten-id blocks, one block per level.
        if ((u32)(level - 1) < 0xA) {
            for (s32 id = (level - 1) * 10 + 1; id <= level * 10; id++) {
                if (!JiggyRelocated(id) && jiggyscore_isCollected((enum jiggy_e)id)) {
                    total++;
                }
            }
            for (int g = 0; g < sJiggyRelocationCount; g++) {
                if (sJiggyRelocations[g].toLevel != level) {
                    continue;
                }
                for (int i = 0; i < sJiggyRelocations[g].count; i++) {
                    total += jiggyscore_isCollected((enum jiggy_e)sJiggyRelocations[g].ids[i]) != 0;
                }
            }
        }

        *result = total;
        *should = false;
    });
}

// Map transitions
const TransitionPair* sTransitionPairs = nullptr;
int sTransitionPairCount = 0;
s32 sTransitionFromMap = 0;

const TransitionPair* FindTransitionPair(s32 from, s32 to) {
    for (int i = 0; i < sTransitionPairCount; i++) {
        const TransitionPair& pair = sTransitionPairs[i];
        if ((pair.from == 0 || pair.from == from) && (pair.to == 0 || pair.to == to)) {
            return &pair;
        }
    }
    return nullptr;
}

void ApplyTransitionPairs() {
    D_8036C150[3].state = 7; // TRANSITION_STATE_7_WHITE_IN
    D_8036C150[3].next_state = 0;
    D_8036C150[3].duration = 0.7f;
    D_8036C150[3].model_index = 0;
    D_8036C150[3].scale = 0.0f;
    D_8036C150[12].duration = 0.4f;
    D_8036C150[13].state = 3;      // TRANSITION_STATE_3_BLACK_OUT
    D_8036C150[13].next_state = 1; // TRANSITION_STATE_1_LOADING
    D_8036C150[13].duration = 0.4f;
    D_8036C150[13].model_index = 0;
    D_8036C150[13].scale = 0.0f;

    COND_VB_SHOULD(VB_MAP_TRANSITION_OUT_INDEX, EVENT_PRIORITY_NORMAL, sTransitionPairCount > 0, {
        const s32 from = va_arg(args, s32);
        const s32 to = va_arg(args, s32);
        s32* outIndex = va_arg(args, s32*);
        sTransitionFromMap = from;
        if (const TransitionPair* pair = FindTransitionPair(from, to)) {
            *outIndex = pair->out;
        }
        (void)should;
    });

    COND_VB_SHOULD(VB_MAP_TRANSITION_IN_INDEX, EVENT_PRIORITY_NORMAL, sTransitionPairCount > 0, {
        const s32 map = va_arg(args, s32);
        s32* inIndex = va_arg(args, s32*);
        const TransitionPair* pair = sTransitionFromMap != 0 ? FindTransitionPair(sTransitionFromMap, map) : nullptr;
        if (pair != nullptr) {
            *inIndex = pair->in;
        }
        (void)should;
    });
}

const SpawnRewrite* sSpawnRewrites = nullptr;
int sSpawnRewriteCount = 0;

RegisterShipInitFunc noteSignInitFunc(ApplyNoteSignHooks, { CVAR_NOTE_RETENTION });
RegisterShipInitFunc pauseNameCenterInit(ApplyPauseNameCentering, { "BOOT" });

} // namespace

void HackShared_EnableNoteSignSuppression(int signActorId) {
    sNoteSignActorId = signActorId;
    ApplyNoteSignHooks();
}

void HackShared_EnableBottlesExplainerSuppression() {
    sSuppressBottlesExplainer = true;
    ApplyNoteSignHooks();
}

void HackShared_EnableDialogSuppression(const int* dialogIds, int count) {
    sSuppressedDialogs = dialogIds;
    sSuppressedDialogCount = count;
    ApplyDialogSuppression();
}

void HackShared_EnableForceAbilitiesUsed(const ability_used_e* moves, int count) {
    for (int i = 0; i < count; i++) {
        sForcedUsedAbilities |= (1 << moves[i]);
    }
    ApplyForceAbilitiesUsed();
}

void HackShared_EnableJiggyRelocation(const JiggyRelocation* groups, int groupCount, const int* alsoExcluded,
                                      int excludedCount) {
    sJiggyRelocations = groups;
    sJiggyRelocationCount = groupCount;
    sJiggyExcluded = alsoExcluded;
    sJiggyExcludedCount = excludedCount;
    ApplyJiggyRelocation();
}

void HackShared_EnableWarpMusicGroups(const WarpMusicGroup* groups, int groupCount) {
    sWarpMusicGroups = groups;
    sWarpMusicGroupCount = groupCount;
    ApplyWarpMusicGroups();
}

void HackShared_EnableTransitionPairs(const TransitionPair* pairs, int count) {
    sTransitionPairs = pairs;
    sTransitionPairCount = count;
    ApplyTransitionPairs();
}

void HackShared_EnableSpawnRewrites(const SpawnRewrite* rewrites, int count) {
    sSpawnRewrites = rewrites;
    sSpawnRewriteCount = count;
}

extern "C" void romhack_RewriteActorSpawn(void* actorInfo, u32* flags) {
    ActorInfo* info = (ActorInfo*)actorInfo;
    for (int i = 0; i < sSpawnRewriteCount; i++) {
        const SpawnRewrite& rewrite = sSpawnRewrites[i];
        if (rewrite.actorId != info->actorId) {
            continue;
        }
        *flags = (*flags & ~rewrite.clearFlags) | rewrite.setFlags;
        if (rewrite.drawDistance != 0) {
            info->draw_distance = rewrite.drawDistance;
        }
        break;
    }
}
