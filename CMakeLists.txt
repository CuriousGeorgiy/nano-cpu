cmake_minimum_required(VERSION 3.17)
project(nano-cpu)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable(processor)
target_sources(processor PUBLIC ProcessorMain.cpp Processor.cpp File.cpp Canary.cpp)
target_include_directories(processor PUBLIC "${PROJECT_SOURCE_DIR}")

add_executable(assembler)
target_sources(assembler PUBLIC AssemblerMain.cpp Assembler.cpp File.cpp Text.cpp)
target_include_directories(assembler PUBLIC "${PROJECT_SOURCE_DIR}")

add_executable(disassembler)
target_sources(disassembler PUBLIC DisassemblerMain.cpp Disassembler.cpp File.cpp)
target_include_directories(disassembler PUBLIC "${PROJECT_SOURCE_DIR}")
