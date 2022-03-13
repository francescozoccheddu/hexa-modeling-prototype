#include <hexa-modeling-prototype/grid.hpp>
#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include <cstdlib>

int main()
{
	Grid grid;
	cinolib::GLcanvas gui;
	cinolib::VolumeMeshControls<MeshGrid> menu(&grid.mesh, &gui);
	gui.push(&grid.mesh);
	gui.push(&menu);
	gui.launch();
	return EXIT_SUCCESS;
}