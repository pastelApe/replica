cmake_minimum_required(VERSION 3.25)

project(replica)

set(CMAKE_CXX_STANDARD 23)

include_directories(include submodule)

add_executable(replica
        src/main.cpp
        include/replica/arguments/arguments.h
        include/replica/arguments/arguments.cpp
        include/replica/copy/copy.h
        include/replica/iou/ring/ring.cpp
        include/replica/iou/ring/ring.h
        include/replica/file/file.h
        include/replica/file/file.cpp
        include/replica/iou/data/data.h
        include/replica/iou/data/data.cpp)

target_link_libraries(replica fmt uring)
