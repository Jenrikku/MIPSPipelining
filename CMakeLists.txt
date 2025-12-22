cmake_minimum_required(VERSION 4.0)
project(mipspipeline)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(FLEX)
flex_target(scanner src/scanner.l ${CMAKE_BINARY_DIR}/scanner.cpp OPTIONS -+)

find_package(BISON)
bison_target(parser src/parser.yy ${CMAKE_BINARY_DIR}/parser.cpp OPTIONS -d -Wcounterexamples)

add_flex_bison_dependency(scanner parser)

add_executable(mipspipeline ${BISON_parser_OUTPUTS} ${FLEX_scanner_OUTPUTS}
                            src/main.cpp
                            src/simulator.cpp
                            src/translator.cpp)

target_include_directories(mipspipeline PRIVATE include)
