//
// Created by 吴文泽 on 2025/11/18.
//

#ifndef FACEDETECTION_SAFEQUEUE_H
#define FACEDETECTION_SAFEQUEUE_H

#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <optional>
#include <iostream>
#include <condition_variable>

using std::cout;

template<typename T>
class SafeQueue {
private:
    std::queue<T> data_queue;
    std::condition_variable cv_empty;
    std::condition_variable cv_full;
    std::atomic_bool pause_request;
    std::atomic_bool stop_request;
    std::mutex q_mutex;
    const int max_size;
    const int size_warning_line;

public:
    explicit SafeQueue(const int max_size)
        : pause_request(false),
          stop_request(false),
          max_size(max_size),
          size_warning_line(static_cast<int>(max_size * 0.8)) {
    }

    bool push(T &&value) {
        std::unique_lock<std::mutex> unique_lock(q_mutex);
        if (stop_request.load(std::memory_order_acquire)) {
            return false;
        }

        if (pause_request.load(std::memory_order_acquire)) {
            return false;
        }

        if (data_queue.size() > max_size) {
            cv_full.wait(unique_lock, [this]() {
                return stop_request.load(std::memory_order_acquire) |
                       pause_request.load(std::memory_order_acquire) |
                       data_queue.size() < max_size;
            });

            if (stop_request.load(std::memory_order_acquire)) {
                return false;
            }
        } else if (data_queue.size() > size_warning_line && data_queue.size() < max_size) {
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        }

        data_queue.push(std::move(value));
        cv_empty.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> unique_lock(q_mutex);
        if (stop_request.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        if (pause_request.load(std::memory_order_acquire)) {
            return std::nullopt;
        }
        if (data_queue.empty()) {
            cv_empty.wait(unique_lock, [this] {
                return stop_request.load(std::memory_order_acquire) |
                       pause_request.load(std::memory_order_acquire) |
                       !data_queue.empty();
            });

            if (stop_request.load(std::memory_order_acquire)) {
                return std::nullopt;
            }
        }

        std::optional<T> value = std::move(data_queue.front());
        data_queue.pop();
        cv_full.notify_one();
        return value;
    }

    size_t size() {
        return data_queue.size();
    }

    bool empty() {
        return data_queue.empty();
    }
};

#endif //FACEDETECTION_SAFEQUEUE_H
