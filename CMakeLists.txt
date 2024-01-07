cmake_minimum_required(VERSION 3.28)
project(NodeScheduler CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set debug
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-deprecated-declarations -Wno-unused-private-field -Wno-unused-command-line-argument -Wno-mismatched-tags -Wno-unused-but-set-variable")

# set sanitizers
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer")

# add libc++ and libc++abi
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
# Export compile commands for use with clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch the spdlog library
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(FetchSPDLog)
include(FetchCXLSHM)
include(FetchLightning)
include(FindLibBpf)

add_subdirectory(src)