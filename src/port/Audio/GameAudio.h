#pragma once
#include <thread>
#include <condition_variable>
static struct AudioState {
    std::thread thread;
    std::condition_variable cv_to_thread, cv_from_thread;
    std::mutex mutex;
    bool running = false;
    bool processing = false;

    void shutdown() {
        if (thread.joinable()) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                running = false;
            }
            cv_to_thread.notify_all();
            thread.join();
        }
    }

    ~AudioState() {
        shutdown();
    }
} audio;

inline void AudioExit() {
    audio.shutdown();
}
