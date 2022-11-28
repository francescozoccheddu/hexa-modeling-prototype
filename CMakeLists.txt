cmake_minimum_required (VERSION 3.14)

include (FetchContent)
set (FETCHCONTENT_QUIET FALSE)

project ("hexa-modeling-prototype")

option (HMP_GUI_ENABLE_DAG_VIEWER "Enable dag viewer widget" OFF)
option (HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER "Enable ae-3d2shape exporter" OFF)
option (HMP_GUI_LIGHT_THEME "Enable GUI light theme" OFF)

# cinolib
set (CINOLIB_HEADER_ONLY OFF)
set (CINOLIB_USES_OPENGL_GLFW_IMGUI ON)
FetchContent_Declare (
    cinolib
    GIT_REPOSITORY "https://github.com/francescozoccheddu/cinolib.git"
    GIT_TAG origin/master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable (cinolib)

# ogdf
if (HMP_GUI_ENABLE_DAG_VIEWER)
    FetchContent_Declare (
        ogdf
        GIT_REPOSITORY "https://github.com/ogdf/ogdf.git"
        GIT_TAG dogwood-202202
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable (ogdf)
endif()

# updatable_priority_queue
FetchContent_Declare (
    updatable_priority_queue
    GIT_REPOSITORY "https://github.com/Ten0/updatable_priority_queue.git"
    GIT_TAG master
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable (updatable_priority_queue)

# cpputils
FetchContent_Declare (
    cpputils
    GIT_REPOSITORY "https://github.com/francescozoccheddu/cpputils.git"
    GIT_TAG origin/master
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable (cpputils)

# hexa-modeling-prototype
add_subdirectory ("core")
add_subdirectory ("gui")