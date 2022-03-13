#include <hexa-modeling-prototype/hello_world.hpp>
#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>

namespace HMP::HelloWorld
{

	void launch()
	{
		using namespace cinolib;
		DrawableHexmesh<> m(CINOLIB_DIR "/examples/data/rockerarm.mesh");
		GLcanvas gui;
		VolumeMeshControls<DrawableHexmesh<>> menu(&m, &gui);
		gui.push(&m);
		gui.push(&menu);
		gui.launch();
	}

}