cmake_minimum_required(VERSION 3.10.2)

set(This CLI)

# set the project name
project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

enable_testing()

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(Headers
    src/CLI.hpp
    
    src/lexparse/impl/cli_lexer_impl.hpp
    src/lexparse/cli_lexer.hpp

    src/exec/impl/cli_exec_impl.hpp
    src/exec/cli_exec.hpp
)

set(Sources
    src/CLI.cpp
    src/lexparse/impl/cli_lexer_impl.cpp
    src/lexparse/cli_lexer.cpp
    src/exec/cli_exec.cpp
)

add_library(CLI_lib STATIC ${Sources} ${Headers})
add_executable(${This} ${Sources} ${Headers})

add_subdirectory(test)
