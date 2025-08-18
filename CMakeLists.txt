cmake_minimum_required(VERSION 3.16)
project(sapotadb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(sapota_engine
    src/engine/engine.cpp
)

target_include_directories(sapota_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# CLI binary
add_executable(sapota_cli
    src/cli/main.cpp
)

target_link_libraries(sapota_cli PRIVATE sapota_engine)