cmake_minimum_required(VERSION 3.17)
project(nano-cpu)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable(processor)
target_sources(processor PUBLIC ProcessorMain.cpp Processor.cpp Processor.hpp file.cpp memory_alloc.cpp canary.cpp log.cpp)
target_include_directories(processor PUBLIC "${PROJECT_SOURCE_DIR}")

add_executable(assembler)
target_sources(assembler PUBLIC AssemblerMain.cpp Assembler.cpp log.cpp Text.cpp canary.cpp memory_alloc.cpp)
target_include_directories(assembler PUBLIC "${PROJECT_SOURCE_DIR}")

add_executable(disassembler)
target_sources(disassembler PUBLIC DisassemblerMain.cpp Disassembler.cpp file.cpp log.cpp)
target_include_directories(disassembler PUBLIC "${PROJECT_SOURCE_DIR}")
