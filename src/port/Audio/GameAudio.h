#pragma once
#include <thread>
#include <atomic>

static struct AudioState {
    std::thread thread;
    std::atomic<bool> running{ false };
    std::atomic<bool> ready{ false };

    void shutdown() {
        if (thread.joinable()) {
            running = false;
            thread.join();
        }
        ready = false;
    }

    ~AudioState() {
        shutdown();
    }
} audio;

inline void AudioExit() {
    audio.shutdown();
}
