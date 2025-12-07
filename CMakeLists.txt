cmake_minimum_required(VERSION 3.16)
project(MiniGit VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

add_executable(minigit
        src/main.cpp
        src/CommitManager.cpp
        src/HashingHelper.cpp
        src/Logviewer.cpp
        src/Repository.cpp
        src/Restore.cpp
        src/CommitNode.cpp
        src/HashTable.cpp
)
