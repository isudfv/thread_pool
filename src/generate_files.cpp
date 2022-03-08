//
// Created by isudfv on 2022/03/08.
//

#include <iostream>
#include <random>
#include <filesystem>
#include <fstream>
#include "md5.h"

int main() {
    using namespace std;
    namespace fs = std::filesystem;
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);
    fs::path path("C:/Users/isudfv/Pictures/files");
    for (int i = 0; i < 163840; ++ i) {
        char buf[1024 * 64 + 1];
//        vector<uint64_t> buf(1024);
/*
        for (auto &j : buf) {
            j = dis(mt);
        }
*/
        for (int j = 0; j < 1024 * 64; j += 64) {
            auto p = dis(mt);
            memcpy(buf + j, &p, 8);
        }
        string str = md5(buf, 1024*64);
//        cout << str << endl;
        ofstream file(path/str, ios::binary);
        file.write(buf, 1024 * 64);
    }
}
