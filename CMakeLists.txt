cmake_minimum_required(VERSION 3.5)
project(mipspipeline)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(mipspipeline src/main.cpp
                            src/interpreter/executer.cpp
                            src/interpreter/parser.cpp)

target_include_directories(mipspipeline PRIVATE include)
