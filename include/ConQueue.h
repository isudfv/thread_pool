//
// Created by isudfv on 2022/03/03.
//
#include <queue>
#include <mutex>
#include <condition_variable>

#ifndef THREAD_POOL_CONQUEUE_H
#define THREAD_POOL_CONQUEUE_H

template <typename T>
class ConQueue {
private:
    std::queue<T> _q;
    std::mutex _m;
public:
    bool empty() {
        std::unique_lock<std::mutex> lock(_m);
        return _q.empty();
    }

    int size() {
        std::unique_lock<std::mutex> lock(_m);
        return _q.size();
    }

    void push(const T& t) {
        std::unique_lock<std::mutex> lock(_m);
        _q.push(t);
        return;
    }

    void pop() {
        std::unique_lock<std::mutex> lock(_m);
        _q.pop();
        return;
    }

    T& front() {
        std::unique_lock<std::mutex> lock(_m);
        return _q.front();
    }
};

#endif //THREAD_POOL_CONQUEUE_H
