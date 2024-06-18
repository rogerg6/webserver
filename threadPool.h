#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

// 单个任务
struct task_t {
    void (*func)(void *);
    void *args;
};

class ThreadPool {
public:
    ThreadPool(int n_threads = 1)
        : threads_num_(n_threads) {}

    ~ThreadPool() {
        is_running_ = false;
        // brodcast all threads
        cond_.notify_all();
        for (int i = 0; i < threads_num_; i++) threads_[i].join();
    }

    ThreadPool(const ThreadPool &other)            = delete;
    ThreadPool &operator=(const ThreadPool &other) = delete;

    void AddTask(const task_t task) {
        std::lock_guard<std::mutex> lk(mtx_);
        q_.push(std::move(task));
        cond_.notify_one();
    }

    void Start() {
        is_running_ = true;
        for (int i = 0; i < threads_num_; i++) {
            std::thread t(&ThreadPool::TaskLoop, this);
            threads_.push_back(std::move(t));
        }
    }

    void TaskLoop() {
        while (is_running_) {
            task_t task;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cond_.wait(lk, [this] { return !this->is_running_ || !q_.empty(); });
                if (!is_running_) break;
                task = std::move(q_.front());
                q_.pop();
            }
            (*task.func)(task.args);
        }
    }

private:
    int                      threads_num_ = 1;
    std::vector<std::thread> threads_;
    bool                     is_running_ = false;
    std::queue<task_t>       q_;
    std::mutex               mtx_;
    std::condition_variable  cond_;
};


#endif