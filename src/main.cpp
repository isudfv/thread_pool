#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <filesystem>
#include "ConQueue.h"
#include "md5.h"
//#include "ThreadPool.h"
#include "thread_pool.h"
#include <fmt/core.h>
#include <fmt/chrono.h>

int main() {
    namespace fs = std::filesystem;
    std::queue<std::string> files;
    for (auto & p : fs::directory_iterator("E:/")) {
        if (p.is_regular_file() && p.file_size() < 10 * 1024 * 1024 * 1024ll) {
            files.push(p.path().string());
        }
    }


/*
    ConQueue<int> q;
//    std::queue<int> q;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution dist(-10000, 10000);
    auto call = [&]() {
        q.push(dist(mt));
//        std::cout << q.front() << std::endl;
        auto get = q.front();
        q.pop();
        printf("%d\n", get);
    };
*/
    auto call = [&] {
        auto p = files.front();
        files.pop();
        fmt::print("{}\n", p);
        auto op = std::chrono::high_resolution_clock::now();
        fmt::print("md5file: {}\n", md5file(p.c_str()).c_str());
        auto ed = std::chrono::high_resolution_clock::now();
        fmt::print("file took {}\n", std::chrono::duration_cast<std::chrono::seconds>(ed - op));
//        std::cout << std::chrono::duration_cast<std::chrono::seconds>(ed - op).count();
    };
//    std::jthread jt(call);

    ThreadPool tp(4);
//    ThreadPool tp(4);
    tp.init();
/*
    while (!files.empty()) {
        auto future = tp.submit(call);
        future.get();
    }
*/
/*
    std::vector<std::thread> n;
    n.reserve(100);
    for (int i = 0; i < 1; ++ i) {
        n.emplace_back(call);
    }
    for (auto &i : n) {
        i.join();
    }
*/
//    for (auto &p : n)
//        p.join();
    return 0;
}
