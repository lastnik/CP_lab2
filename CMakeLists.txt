cmake_minimum_required(VERSION 3.14)
project(kuznechik)

set(CMAKE_CXX_STANDARD 17)

add_executable(kuznechik main.cpp cipher.cpp cipher.h block.cpp block.h Logger.cpp Logger.h Exeptions.h Buffer.cpp Buffer.h)