cmake_minimum_required(VERSION 3.16)
project(filesorter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

set(SOURCE_FILES
    src/main.cpp
    src/ConfigLoader.cpp
    src/FileWatcher.cpp
    src/FileSorter.cpp
)

add_executable(filesorter ${SOURCE_FILES})

target_link_libraries(filesorter
    PRIVATE
    pthread
)