cmake_minimum_required (VERSION 3.8)

add_executable (gui 
    "src/main.cpp" 
)

target_include_directories (gui
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include"
)

target_link_libraries (gui 
    PRIVATE core
)