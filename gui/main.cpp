#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>

int main()
{
    using namespace cinolib;
    // Hello world mesh
    DrawableHexmesh<> m(CINOLIB_DIR "/examples/data/rockerarm.mesh");
    GLcanvas gui;
    VolumeMeshControls<DrawableHexmesh<>> menu(&m, &gui);
    gui.push(&m);
    gui.push(&menu);
    return gui.launch();
}