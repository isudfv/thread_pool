cmake_minimum_required(VERSION 3.20)
project(thread_pool)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(fmt CONFIG REQUIRED)

add_executable(thread_pool src/main.cpp src/md5.cpp)

target_link_libraries(thread_pool PRIVATE fmt::fmt-header-only)
