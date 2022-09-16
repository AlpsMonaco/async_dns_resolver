add_executable(test_dns_query
    ${PROJECT_SOURCE_DIR}/tests/test_dns_query.cpp
)

target_include_directories(test_dns_query
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(UNIX)
    target_link_libraries(test_dns_query
        PRIVATE
            pthread
    )
endif()

target_link_libraries(test_dns_query
    PRIVATE
        dns_resolve_library
)

add_executable(test_cares_features
    ${PROJECT_SOURCE_DIR}/tests/test_cares_features.cpp
)

target_include_directories(test_cares_features
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(UNIX)
    target_link_libraries(test_cares_features
        PRIVATE
            pthread
    )
endif()

target_link_libraries(test_cares_features
    PRIVATE
        c-ares::cares
)
