cmake_minimum_required(VERSION 3.6)
project(ass1)

set(CMAKE_C_STANDARD 99)

set(COMMON_SOURCE_FILES Socket.c ChatProtocol.c)
set(CLIENT_SOURCE_FILES client.c)
set(SERVER_SOURCE_FILES server.c VoidList.c ClientList.c ClientList.h UserList.c UserList.h MessageList.c MessageList.h BlockedList.c BlockedAddrList.c BlockedAddrList.h)

add_executable(client ${COMMON_SOURCE_FILES} ${CLIENT_SOURCE_FILES})
add_executable(server ${COMMON_SOURCE_FILES} ${SERVER_SOURCE_FILES})

if (WIN32)
    target_link_libraries(client wsock32.lib)
    target_link_libraries(client Ws2_32.lib)
endif()