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
//        int _id;
        ThreadPool *_pool;
    public:
        explicit ThreadWorker(ThreadPool *pool/*, const int id*/) : _pool(pool)/*, _id(id)*/ {}

        void operator () () {
//            std::function<void ()> func;

            bool success = false;
            while (!_pool->_shutdown) {
                std::function<void()> func;
                {
                    std::unique_lock<std::mutex> lock(_pool->_mutex); // 单个线程获取任务时加锁

                    if (_pool->_q.empty()){
                        std::cout << "blocked\n";
                        _pool->_cv.wait(lock);
                    }

                    func = _pool->_q.front();
                    _pool->_q.pop();
                    success = true;
                }

                if (success)
                    func();
            }
        }
    };

public:
    explicit ThreadPool(const int _n_threads) : _shutdown(false) {
        _threads.resize(_n_threads);
//        for (int i = 0; i < _threads.size(); ++ i)
        for (auto & p : _threads) {
            // 初始化线程，任务队列目前为空，自然阻塞
            p = std::jthread(ThreadWorker(this));
        }
    }

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool(ThreadPool &&) = delete;

    ThreadPool &operator = (const ThreadPool &) = delete;

    ThreadPool &operator = (ThreadPool &&) = delete;

    void shutdown() {
        _shutdown = true;
        // 唤醒并 join 所有线程
        _cv.notify_all();

        for (auto &p : _threads) {
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
        std::function<void ()> wrapper_func = [task_ptr] {
            (*task_ptr)();
        };
        // 添加到任务队列
        _q.push(wrapper_func);
        // 唤醒一个线程
        _cv.notify_one();

        return task_ptr->get_future();
    }

};


#endif //THREAD_POOL_THREADPOOL_H
