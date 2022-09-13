cmake_minimum_required(VERSION 3.13)

project(dns_query)

set (CMAKE_CXX_STANDARD 17)

add_subdirectory(third_party/c-ares)

add_library(DNS_QUERY_LIBRARY
    ${PROJECT_SOURCE_DIR}/include/dns_query/dns_query.h
    ${PROJECT_SOURCE_DIR}/include/dns_query/dns_query.cpp
)

set_target_properties(DNS_QUERY_LIBRARY PROPERTIES OUTPUT_NAME "dns_query_library")

target_link_libraries(DNS_QUERY_LIBRARY
    PRIVATE
        c-ares::cares
)

add_executable(DNS_QUERY_TEST
    ${PROJECT_SOURCE_DIR}/tests/test_dns_query.cpp
)

target_include_directories(DNS_QUERY_TEST
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(DNS_QUERY_TEST
    PRIVATE
        DNS_QUERY_LIBRARY
)