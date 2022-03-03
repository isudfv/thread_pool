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
    ConQueue<std::function<void()>> _q;
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

            while (!_pool->_shutdown) {
                std::unique_lock<std::mutex> lock(_pool->_mutex);

                if (_pool->_q.empty()){
                    std::cout << "blocked\n";
                    _pool->_cv.wait(lock);
                }

                if (!_pool->_q.empty()) {
                    auto func = _pool->_q.front();
                    _pool->_q.pop();
                    func();
                }
            }
        }
    };

public:
    explicit ThreadPool(const int _n_threads) : _shutdown(false) {
        _threads.resize(_n_threads);
//        for (int i = 0; i < _threads.size(); ++ i)
        for (auto & p : _threads) {
            p = std::jthread(ThreadWorker(this));
        }
    }

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool(ThreadPool &&) = delete;

    ThreadPool &operator = (const ThreadPool &) = delete;

    ThreadPool &operator = (ThreadPool &&) = delete;

    void shutdown() {
        _shutdown = true;
        _cv.notify_all();

        //todo join
    }

    template<typename F, typename ...Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        std::function<void ()> wrapper_func = [task_ptr] {
            (*task_ptr)();
        };

        _q.push(wrapper_func);

        _cv.notify_one();

        return task_ptr->get_future();
    }

};


#endif //THREAD_POOL_THREADPOOL_H
