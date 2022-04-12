cmake_minimum_required (VERSION 3.8)

add_executable (gui 
    "src/main.cpp" 
    "src/dag/Viewer.cpp"
    "src/dag/Layout.cpp"
    "src/App.cpp"
    "src/dag/createLayout.cpp"
)

set_target_properties (gui PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

target_include_directories (gui
    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include"
)

target_link_libraries (gui 
    PRIVATE core
    PRIVATE OGDF
)

