cmake_minimum_required(VERSION 3.16)
project(untitled)

set(CMAKE_CXX_STANDARD 17)

# Homebrew paths for ARM (M1, M2, M3, M4)
set(BREW_PREFIX "/opt/homebrew")

include_directories(
        ${BREW_PREFIX}/include
        ${BREW_PREFIX}/opt/libpq/include
        ${BREW_PREFIX}/opt/libpqxx/include
)

link_directories(
        ${BREW_PREFIX}/lib
        ${BREW_PREFIX}/opt/libpq/lib
        ${BREW_PREFIX}/opt/libpqxx/lib
)

add_executable(untitled main.cpp)

target_link_libraries(untitled
        pq
        pqxx
)


