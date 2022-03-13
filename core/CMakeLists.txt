cmake_minimum_required (VERSION 3.8)

add_library (core STATIC
    "src/hello_world.cpp"
    "include/hexa-modeling-prototype/hello_world.hpp"
)

target_include_directories (core
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include"
)

target_link_libraries (core 
    PRIVATE cinolib
)