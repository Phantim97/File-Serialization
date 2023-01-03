cmake_minimum_required(VERSION 3.0.0)

project("File Serializer" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FileSerializer test.cpp)