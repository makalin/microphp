cmake_minimum_required(VERSION 3.20)
project(microphp VERSION 0.1.0 LANGUAGES C CXX)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(MICROPHP_EXCEPTIONS "Enable exception handling" ON)
option(MICROPHP_FLOAT64 "Use 64-bit floating point" ON)
option(MICROPHP_STDIO "Enable stdio support" ON)
option(MICROPHP_NET "Enable networking support" OFF)
option(MICROPHP_KVSTORE "Enable key-value store" ON)

# Memory configuration
set(MICROPHP_STR_ARENA_KB 128 CACHE STRING "String arena size in KB")
set(MICROPHP_ARRAY_ARENA_KB 128 CACHE STRING "Array arena size in KB")
set(MICROPHP_STACK_KB 24 CACHE STRING "Stack size in KB")
set(MICROPHP_TASKS_MAX 4 CACHE STRING "Maximum number of tasks")

# Add subdirectories
add_subdirectory(core)
add_subdirectory(tools)
add_subdirectory(targets)

# Print configuration
message(STATUS "micro-PHP Configuration:")
message(STATUS "  Exceptions: ${MICROPHP_EXCEPTIONS}")
message(STATUS "  Float64: ${MICROPHP_FLOAT64}")
message(STATUS "  Stdio: ${MICROPHP_STDIO}")
message(STATUS "  Network: ${MICROPHP_NET}")
message(STATUS "  KV Store: ${MICROPHP_KVSTORE}")
message(STATUS "  String Arena: ${MICROPHP_STR_ARENA_KB} KB")
message(STATUS "  Array Arena: ${MICROPHP_ARRAY_ARENA_KB} KB")
message(STATUS "  Stack: ${MICROPHP_STACK_KB} KB")
message(STATUS "  Max Tasks: ${MICROPHP_TASKS_MAX}")
