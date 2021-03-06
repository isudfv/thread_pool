#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <filesystem>
#include "ConQueue.h"
#include "md5.h"
#include "ThreadPool.h"
#include <fmt/core.h>
#include <fmt/chrono.h>

int main() {
    using namespace std::literals;
    namespace fs = std::filesystem;
//    std::queue<std::string> files;
    std::vector<std::string> files;
    fs::path path("C:/Users/isudfv/Pictures/files");
    auto op = std::chrono::high_resolution_clock::now();
    for (auto &p: fs::directory_iterator(path)) {
        files.push_back(p.path().string());
    }
    auto ed = std::chrono::high_resolution_clock::now();
//    fmt::print("vector took {}\n", std::chrono::duration_cast<std::chrono::seconds>(ed - op));
    fmt::print("vector took {:.2f}s\n",
               std::chrono::duration_cast<std::chrono::milliseconds>(ed - op).count() / 1000.0);

    auto call = [](const std::string &name) {
        auto op = std::chrono::high_resolution_clock::now();
        auto md5 = fmt::format("{}", md5file(name.c_str()).c_str());
        auto ed = std::chrono::high_resolution_clock::now();

        return std::make_tuple(name.substr(name.rfind('\\') + 1),
                               md5,
                               std::chrono::duration_cast<std::chrono::seconds>(ed - op));
    };

    op = std::chrono::high_resolution_clock::now();

//    ThreadPool tp(2);
    ThreadPool tp(16);

    std::vector<std::future<decltype(call(""))>> futures;
    futures.reserve(files.size());
//    while (!files.empty()) {
    for (auto &&name: files) {
//        auto name = files.front();
//        files.pop();
        futures.emplace_back(tp.submit(call, name));
//        future.get();
    }
    for (auto &f: futures) {
        auto[name, md5, time] = f.get();
//        fmt::print("name: {}\nmd5:  {}\ntook: {}\n", name, md5, time);
    }
//    std::this_thread::sleep_for(1s);
    tp.shutdown();
    ed = std::chrono::high_resolution_clock::now();
    fmt::print("16 threads md5 took {:.2f}s\n",
               std::chrono::duration_cast<std::chrono::milliseconds>(ed - op).count() / 1000.0);

    return 0;
}
