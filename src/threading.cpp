#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <vector>
#include <atomic>

template<typename T>
class ThreadSafeQueue {
public:
    void push(T item) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(std::move(item));
        }
        cv_.notify_one();
    }

    // Returns false if queue was shut down and empty
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { 
            return !queue_.empty() || done_; 
        });
        if (queue_.empty()) 
        {
            return false;
        } 
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            done_ = true;
        }
        cv_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool done_ = false;
};

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] { workerLoop(); });
        }
    }

    void submit(std::function<void()> task) {
        tasks_.push(std::move(task));
    }

    ~ThreadPool() {
        tasks_.shutdown();
        for (auto& t : workers_) t.join();
    }

private:
    void workerLoop() {
        std::function<void()> task;
        while (tasks_.pop(task)) {
            task(); // do the work in parallel
        }
    }

    ThreadSafeQueue<std::function<void()>> tasks_;
    std::vector<std::thread> workers_;
};