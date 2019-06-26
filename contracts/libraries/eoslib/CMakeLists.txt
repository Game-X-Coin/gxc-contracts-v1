cmake_minimum_required(VERSION 3.5)

project(eoslib)

add_library(eoslib STATIC
   src/crypto.cpp
)

target_include_directories(eoslib PUBLIC include)

add_subdirectory(lib)
