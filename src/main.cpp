#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <filesystem>
#include "ConQueue.h"
#include "md5.h"
#include "ThreadPool.h"
//#include "thread_pool.h"
#include <fmt/core.h>
#include <fmt/chrono.h>

int main() {
    using namespace std::literals;
    namespace fs = std::filesystem;
    std::queue<std::string> files;
    for (auto & p : fs::directory_iterator("C:/Users/isudfv/Pictures/Sono.Bisque.Doll.wa.Koi.wo.Suru.2022.Crunchyroll.WEB-DL.1080p.x264.AAC-HDCTV")) {
        if (p.is_regular_file() && p.file_size() < 10 * 1024 * 1024 * 1024ll) {
            files.push(p.path().string());
        }
    }

    auto call = [&files] {
        auto p = files.front();
        files.pop();
        fmt::print("{}\n", p);

        auto op = std::chrono::high_resolution_clock::now();
        fmt::print("md5: {}\n", md5file(p.c_str()).c_str());
        auto ed = std::chrono::high_resolution_clock::now();

        fmt::print("file took {}\n", std::chrono::duration_cast<std::chrono::seconds>(ed - op));
    };

    auto op = std::chrono::high_resolution_clock::now();

//    ThreadPool tp(2);
    ThreadPool tp(4);

//    std::this_thread::sleep_for(2s);
    while (!files.empty()) {
//        call();
        auto future = tp.submit(call);
//        future.get();
    }
    tp.shutdown();
    auto ed = std::chrono::high_resolution_clock::now();
    fmt::print("all took {}\n", std::chrono::duration_cast<std::chrono::seconds>(ed - op));

    return 0;
}
