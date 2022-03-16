cmake_minimum_required (VERSION 3.8)

# cinolib
set (CINOLIB_HEADER_ONLY OFF)
set (CINOLIB_USES_OPENGL_GLFW_IMGUI ON)
set (cinolib_DIR "${PROJECT_SOURCE_DIR}/external/cinolib")
find_package (cinolib REQUIRED)

# core library
add_library (core STATIC
    "src/grid.cpp"
    "src/operationstree.cpp"
    "src/undoredo.cpp"
    "src/utils.cpp"
    "src/actions/extrudeaction.cpp"
    "src/actions/facerefineaction.cpp"
    "src/actions/makeconformingaction.cpp"
    "src/actions/moveaction.cpp"
    "src/actions/refineaction.cpp"
    "src/actions/removeaction.cpp"
    "include/hexa-modeling-prototype/grid.hpp"
    "include/hexa-modeling-prototype/operationstree.hpp"
    "include/hexa-modeling-prototype/refinementschemes.hpp"
    "include/hexa-modeling-prototype/undoredo.hpp"
    "include/hexa-modeling-prototype/utils.hpp"
    "include/hexa-modeling-prototype/actions/extrudeaction.hpp"
    "include/hexa-modeling-prototype/actions/facerefineaction.hpp"
    "include/hexa-modeling-prototype/actions/makeconformingaction.hpp"
    "include/hexa-modeling-prototype/actions/moveaction.hpp"
    "include/hexa-modeling-prototype/actions/refineaction.hpp"
    "include/hexa-modeling-prototype/actions/removeaction.hpp"
)

set_target_properties (core PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

if (MSVC AND CINOLIB_HEADER_ONLY)
    target_compile_options (core PRIVATE /bigobj)
endif()

target_include_directories (core
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include"
)

target_link_libraries (core 
    PUBLIC cinolib
)