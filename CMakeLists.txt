cmake_minimum_required(VERSION 3.1)

project(dns_resolve)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set (CMAKE_CXX_STANDARD 17)

set(CARES_SHARED OFF)
set(CARES_STATIC ON)

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/third_party/c-ares/CMakeLists.txt")
    message(FATAL_ERROR  "third_party/c-ares not exist\nplease run \"git submodule update --init\"")
endif()

add_subdirectory(third_party/c-ares)

add_library(dns_resolve_library
    ${PROJECT_SOURCE_DIR}/include/dns_resolve/dns_resolve.cpp
)

target_include_directories(dns_resolve_library
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include/
)

target_link_libraries(dns_resolve_library
    PRIVATE
        c-ares::cares
)

add_library(dns_resolve::lib ALIAS dns_resolve_library)

add_subdirectory(${PROJECT_SOURCE_DIR}/tests)