cmake_minimum_required (VERSION 3.14)

include (FetchContent)
set (FETCHCONTENT_QUIET FALSE)

project ("hexa-modeling-prototype" VERSION 0.1)

add_compile_definitions(HMP_NAME=\"${CMAKE_PROJECT_NAME}\")
add_compile_definitions(HMP_VERSION=\"${CMAKE_PROJECT_VERSION}\")

option (HMP_GUI_ENABLE_DAG_VIEWER "Enable dag viewer widget" OFF)
option (HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER "Enable ae-3d2shape exporter" OFF)
option (HMP_AGGRESSIVE_DEBUG "Enable ASAN and debug libc" OFF)
option (HMP_AGGRESSIVE_WARNINGS "Enable a lot of warnings" OFF)

# cinolib
set (CINOLIB_HEADER_ONLY OFF)
set (CINOLIB_USES_OPENGL_GLFW_IMGUI ON)
FetchContent_Declare (
    cinolib
    GIT_REPOSITORY "https://github.com/francescozoccheddu/cinolib.git"
    GIT_TAG "e47eca7bd0424e8d333a748b53d436c88e845895"
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable (cinolib)

# ogdf
if (HMP_GUI_ENABLE_DAG_VIEWER)
    FetchContent_Declare (
        ogdf
        GIT_REPOSITORY "https://github.com/ogdf/ogdf.git"
        GIT_TAG "dogwood-202202"
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable (ogdf)
endif()

# updatable_priority_queue
FetchContent_Declare (
    updatable_priority_queue
    GIT_REPOSITORY "https://github.com/Ten0/updatable_priority_queue.git"
    GIT_TAG "8a7facc90855f64ad463d7edd393eff0fc6d97af"
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable (updatable_priority_queue)

# cpputils
FetchContent_Declare (
    cpputils
    GIT_REPOSITORY "https://github.com/francescozoccheddu/cpputils.git"
    GIT_TAG "24d2947832313ba2a71e8c05e1ec2d38ff6e6cc3"
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable (cpputils)

# hexa-modeling-prototype
add_subdirectory ("core")
add_subdirectory ("gui")

# compile options

if (HMP_AGGRESSIVE_DEBUG)
    add_compile_definitions(_GLIBCXX_DEBUG HMP_AGGRESSIVE_DEBUG)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if (HMP_AGGRESSIVE_WARNINGS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wunused)
        target_compile_options(gui PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wunused)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(core PRIVATE /W4)
        target_compile_options(gui PRIVATE /W4)
    endif()
endif()
