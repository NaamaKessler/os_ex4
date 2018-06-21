cmake_minimum_required(VERSION 3.8)
project(os_ex4)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES whatsappClient.cpp whatsappClient.hpp whatsappServer.cpp whatsappServer.hpp whatsappio.h
        whatsappio.cpp)
add_executable(os_ex4 ${SOURCE_FILES})