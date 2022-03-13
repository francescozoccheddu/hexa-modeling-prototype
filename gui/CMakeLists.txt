cmake_minimum_required (VERSION 3.8)

add_executable (gui 
    "main.cpp" 
)

target_link_libraries (gui 
    PRIVATE cinolib
)