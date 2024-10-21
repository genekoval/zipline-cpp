target_sources(zipline PRIVATE
    error.cpp
    error_list.cpp
)

if(PROJECT_TESTING)
    target_sources(zipline.test PRIVATE
        error.test.cpp
        router.test.cpp
        server_protocol.test.cpp
        socket.test.cpp
    )
endif()

add_subdirectory(codable)
