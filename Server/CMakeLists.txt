cmake_minimum_required(VERSION 3.10)
project(server)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
set(SOURCES
	src/server.cpp
	src/server_class.cpp
	src/crypt.cpp
)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
add_executable(server ${SOURCES})
target_include_directories(server  PRIVATE ${BOOST_INCLUDE_DIR})
target_link_libraries(server PRIVATE ${BOOST_LIBRARIES})
target_include_directories(server PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(server PRIVATE ${OPENSSL_LIBRARIES})
