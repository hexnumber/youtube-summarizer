cmake_minimum_required(VERSION 3.16)
project(youtube-summarizer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Help CMake find Qt6 when installed via Homebrew
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt6")
endif()

find_package(CURL REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(yt-summarizer
    src/main.cpp
    src/mainwindow.cpp
    src/worker.cpp
    src/ollama.cpp
)

target_include_directories(yt-summarizer PRIVATE include src)
target_link_libraries(yt-summarizer PRIVATE CURL::libcurl Qt6::Widgets)
