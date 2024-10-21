target_sources(zipline PUBLIC FILE_SET HEADERS FILES
    client.hpp
    error.hpp
    error_list.hpp
    response.hpp
    router.hpp
    server_protocol.hpp
    zipline
)

add_subdirectory(codable)
add_subdirectory(io)
