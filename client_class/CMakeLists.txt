cmake_minimum_required(VERSION 3.10)
project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED )
set(SOURCES
    src/main.cpp
    src/client_class.cpp
    src/crypt.cpp 
  	  
)
add_executable(client ${SOURCES})

target_include_directories(client PRIVATE ${OPENSSL_INCLUDE_DIR})
target_include_directories(client PRIVATE ${BOOST_INCLUDE_DIR})

target_link_libraries(client PRIVATE ${OPENSSL_LIBRARIES})
target_link_libraries(client PRIVATE ${BOOST_LIBRARIES})
