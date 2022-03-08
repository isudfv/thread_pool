//
// Created by isudfv on 2022/03/03.
//

#ifndef THREAD_POOL_THREADPOOL_H
#define THREAD_POOL_THREADPOOL_H

#include "ConQueue.h"
#include <functional>
#include <thread>
#include <future>
#include <utility>
#include <memory>
#include <fmt/core.h>

class ThreadPool {
private:
    bool _shutdown;
    std::queue<std::function<void()>> _q; // 线程获取任务时加了锁，所以不需要线程安全的队列
//    ConQueue<std::function<void()>> _q;
    std::vector<std::jthread> _threads;
    std::mutex _mutex;
    std::condition_variable _cv;

    class ThreadWorker {
    private:
        int _id;
        ThreadPool *_pool;
    public:
        explicit ThreadWorker(ThreadPool *pool, const int id) : _pool(pool), _id(id) {}

        void operator()() {
//            std::function<void ()> func;

            bool success = false;
            while (!_pool->_shutdown) {
                std::function<void()> func;
                {
                    std::unique_lock<std::mutex> lock(_pool->_mutex); // 单个线程获取任务时加锁

                    if (_pool->_q.empty()) {
//                        fmt::print("blocked\n");
                        _pool->_cv.wait(lock);
                    }

                    success = false;
                    if (!_pool->_q.empty()){
                        func = _pool->_q.front();
                        _pool->_q.pop();
                        success = true;
                    }
                }

                if (success)
                    func();
            }
        }
    };

public:
    explicit ThreadPool(const int _n_threads) : _shutdown(false) {
        _threads.resize(_n_threads);
        for (int i = 0; i < _threads.size(); ++i) {
//        for (auto &p: _threads) {
            // 初始化线程，任务队列目前为空，自然阻塞
//            p = std::jthread(ThreadWorker(this, 0));
            _threads[i] = std::jthread(ThreadWorker(this, i));
        }
    }

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool(ThreadPool &&) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

    ThreadPool &operator=(ThreadPool &&) = delete;

    void shutdown() {
        // wait 1ms to insure every thread is blocked
        using namespace std::literals;
        std::this_thread::sleep_for(1ms);
        _shutdown = true;
        // 唤醒并 join 所有线程
        _cv.notify_all();
//        fmt::print("notify all\n");

        for (auto &p: _threads) {
            if (p.joinable())
                p.join();
        }
        //todo join
    }

    template<typename F, typename ...Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        // 包装函数及其参数
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        // 用 std::packaged_task 封装任务以异步操作结果
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        // wrapper是为了统一调用 func()
/*
        std::function<void()> wrapper_func = [task_ptr] {
            (*task_ptr)();
        };
*/
        // 添加到任务队列，用 lambda 替换 wrapper
        {
            std::unique_lock<std::mutex> lock(_mutex);
//            fmt::print("queue size {}\n", this->_q.size());
            _q.emplace([task_ptr] {
                (*task_ptr)();
            });
//            fmt::print("queue size {}\n", this->_q.size());
        }
//        _q.push(wrapper_func);
        // 唤醒一个线程
        _cv.notify_one();

        return task_ptr->get_future();
    }

/*
    template<typename F, typename... A,
            typename = std::enable_if_t<std::is_void_v<
                    std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>>>>
    std::future<bool> Submit(const F &task, const A &&... args) {
        auto promise = std::make_shared<std::promise<bool>>();
        auto future = promise->get_future();
        PushTask([task, args..., promise] {
            task(args...);
            promise->set_value(true);
        });
        return future;
    }
*/

/*
    template<
            typename F, typename... A,
            typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<A>...>,
            typename = std::enable_if_t<!std::is_void_v<R>>>
    std::future<R> Submit(const F &task, const A &&... args) {
        auto promise = std::make_shared<std::promise<R>>();
        auto future = promise->get_future();
        _q.emplace([task, args..., promise] {
            promise->set_value(task(args...));
        });
        return future;
    }

*/
};


#endif //THREAD_POOL_THREADPOOL_H
