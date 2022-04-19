cmake_minimum_required (VERSION 3.8)

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
    "src/Dag/Delete.cpp"
    "src/Dag/Element.cpp"
    "src/Dag/Extrude.cpp"
    "src/Dag/Node.cpp"
    "src/Dag/Operation.cpp"
    "src/Dag/Refine.cpp"
    "src/Refinement/schemes.cpp"
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