cmake_minimum_required(VERSION 3.17)
project(nano-cpu)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(processor)
target_sources(processor PUBLIC processor_main.cpp processor.cpp file.cpp stack.cpp hash_sum.cpp memory_alloc.cpp canary.cpp log.cpp)
target_include_directories(processor PUBLIC "${PROJECT_SOURCE_DIR}")

add_executable(assembler)
target_sources(assembler PUBLIC assembler_main.cpp log.cpp text.cpp canary.cpp memory_alloc.cpp assembler.cpp)
target_include_directories(assembler PUBLIC "${PROJECT_SOURCE_DIR}")

add_executable(disassembler)
target_sources(disassembler PUBLIC disassembler_main.cpp disassembler.cpp file.cpp log.cpp)
target_include_directories(disassembler PUBLIC "${PROJECT_SOURCE_DIR}")