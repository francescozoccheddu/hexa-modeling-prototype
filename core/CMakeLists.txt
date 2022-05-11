cmake_minimum_required (VERSION 3.8)

add_library (core STATIC
    "src/Actions/Clear.cpp"
    "src/Actions/Delete.cpp"
    "src/Actions/Extrude.cpp"
    "src/Actions/Load.cpp"
    "src/Actions/MakeConforming.cpp"
    "src/Actions/MoveVert.cpp"
    "src/Actions/Paste.cpp"
    "src/Actions/Refine.cpp"
    "src/Actions/Rotate.cpp"
    "src/Actions/Utils.cpp"
    "src/Dag/Delete.cpp"
    "src/Dag/Element.cpp"
    "src/Dag/Extrude.cpp"
    "src/Dag/Node.cpp"
    "src/Dag/NodeSet.cpp"
    "src/Dag/Operation.cpp"
    "src/Dag/Refine.cpp"
    "src/Dag/Utils.cpp"
    "src/Meshing/Mesher.cpp"
    "src/Meshing/Refinement.cpp"
    "src/Meshing/refinementSchemes.cpp"
    "src/Meshing/Utils.cpp"
    "src/Utils/Geometry.cpp"
    "src/Utils/Serialization.cpp"
    "src/Commander.cpp"
    "src/Project.cpp"
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
    PUBLIC cpputils
)