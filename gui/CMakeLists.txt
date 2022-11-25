cmake_minimum_required (VERSION 3.14)

add_executable (gui 
    "src/main.cpp" 
    "src/App.cpp"
    "src/Utils/HrDescriptions.cpp"
    "src/Utils/Transform.cpp"
    "src/Utils/Controls.cpp"
    "src/Widgets/Axes.cpp"
    "src/Widgets/Commander.cpp"
    "src/Widgets/Target.cpp"
    "src/Widgets/VertEdit.cpp"
    "src/Widgets/DirectVertEdit.cpp"
    "src/Widgets/Ae3d2ShapeExporter.cpp"
)

set_target_properties (gui PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

target_include_directories (gui
    PRIVATE "include"
)

target_link_libraries (gui 
    PRIVATE core
)

if (HMP_GUI_ENABLE_DAG_VIEWER)

    target_sources(gui
        PRIVATE "src/DagViewer/createLayout.cpp"
        PRIVATE "src/DagViewer/Layout.cpp"
        PRIVATE "src/DagViewer/Widget.cpp"
    )

    target_link_libraries (gui 
        PRIVATE OGDF
    )

    target_compile_definitions(gui 
        PUBLIC HMP_GUI_ENABLE_DAG_VIEWER
    )

endif()