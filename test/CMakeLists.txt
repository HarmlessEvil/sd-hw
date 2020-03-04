cmake_minimum_required(VERSION 3.10.2)

set(This LexParseTests)

set(Sources
    LexParseTests.cpp
)

add_executable(${This} ${Sources})

target_link_libraries(${This} PUBLIC
    ${GTEST_BOTH_LIBRARIES}
    CLI_lib
)

add_test(
    NAME ${This}
    COMMAND ${This}
)
