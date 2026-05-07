cmake_minimum_required(VERSION 3.16)
project(youtube-summarizer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

add_executable(yt-summarizer
    src/main.cpp
)

target_include_directories(yt-summarizer PRIVATE include)
target_link_libraries(yt-summarizer PRIVATE CURL::libcurl)
