cmake_minimum_required(VERSION 3.15)
project(securemux)

set(CMAKE_CXX_STANDARD 17)

# Server binary
add_executable(securemux-server
    src/server/server.cpp
    src/server/connection.cpp
    src/pty.cpp
    src/utils/utils.cpp
)
target_include_directories(securemux-server PRIVATE includes)
target_link_libraries(securemux-server util)



# Client binary
add_executable(securemux
    src/client/client.cpp
    src/client/client-connection.cpp
    src/utils/utils.cpp
)
target_include_directories(securemux PRIVATE includes)