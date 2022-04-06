#include <hexa-modeling-prototype/grid.hpp>
#include <cinolib/meshes/meshes.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include <cstdlib>
#include <iostream>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cinolib/gl/file_dialog_open.h>
#include <cinolib/gl/file_dialog_save.h>

struct
{
	cinolib::vec2d position{};
	unsigned int pid{ 0 };
	bool pointing{ false };
} mouse;

struct
{
	unsigned int vid{};
	bool pending{ false };
} moveOp;

struct
{
	unsigned int pid{};
	bool pending{ false };
} copyOp;

struct
{
	cinolib::DrawableTrimesh<>* p_mesh{ nullptr };
	std::string filename{};
} target;

HMP::Grid grid{};
cinolib::GLcanvas gui{};

void updateMouse(cinolib::vec2d _screen)
{
	mouse.position = _screen;
	unsigned int pid{};
	cinolib::vec3d world;
	bool pointing = gui.unproject(mouse.position, world);
	if (pointing)
	{
		pid = grid.mesh.pick_poly(world);
	}
	if (pointing != mouse.pointing || (pointing && pid != mouse.pid))
	{
		if (mouse.pointing && mouse.pid < grid.mesh.num_polys())
		{
			grid.mesh.poly_data(mouse.pid).color = cinolib::Color::WHITE();
		}
		if (pointing)
		{
			grid.mesh.poly_data(pid).color = cinolib::Color::YELLOW();
		}
		grid.mesh.updateGL();
	}
	mouse.pid = pid;
	mouse.pointing = pointing;
}

void refit(bool keepModel = true)
{
	gui.refit_scene(keepModel);
	updateMouse(mouse.position);
}

int main()
{
	cinolib::VolumeMeshControls<HMP::MeshGrid> menu{ &grid.mesh, &gui, "Grid mesh" };
	gui.push(&grid.mesh);
	gui.push(&menu);

	gui.callback_mouse_moved = [&](double _x, double _y)
	{
		updateMouse(cinolib::vec2d{ _x, _y });
	};

	gui.callback_key_pressed = [&](int _key, int _modifiers)
	{
		switch (_key)
		{
			// move vertex
			case GLFW_KEY_M:
			{
				if (!_modifiers)
				{
					cinolib::vec3d world_mouse_pos;
					if (gui.unproject(mouse.position, world_mouse_pos))
					{
						if (moveOp.pending)
						{
							// FIXME move in plane instead of using world_mouse_pos
							const cinolib::vec3d offset{ world_mouse_pos - grid.mesh.vert(moveOp.vid) };
							grid.move_vert(moveOp.vid, offset);
							moveOp.pending = false;
							refit();
						}
						else
						{
							moveOp.vid = grid.mesh.pick_vert(world_mouse_pos);
							moveOp.pending = true;
						}
					}
					else
					{
						moveOp.pending = false;
					}
				}
			}
			break;
			// extrude
			case GLFW_KEY_E:
			{
				if (!_modifiers)
				{
					cinolib::vec3d world_mouse_pos;
					if (gui.unproject(mouse.position, world_mouse_pos))
					{
						const unsigned int pid = grid.mesh.pick_poly(world_mouse_pos);
						unsigned int closest_fid{};
						{
							double closest_dist = std::numeric_limits<double>::infinity();
							for (const unsigned int fid : grid.mesh.poly_faces_id(pid))
							{
								const double dist{ world_mouse_pos.dist_sqrd(grid.mesh.face_centroid(fid)) };
								if (dist < closest_dist)
								{
									closest_dist = dist;
									closest_fid = fid;
								}
							}
						};
						const unsigned int offset{ grid.mesh.poly_face_offset(pid, closest_fid) };
						grid.add_extrude(pid, offset);
						refit();
					}
				}
			}
			break;
			// copy
			case GLFW_KEY_C:
			{
				if (!_modifiers)
				{
					cinolib::vec3d world_mouse_pos;
					if (gui.unproject(mouse.position, world_mouse_pos))
					{
						const unsigned int pid{ grid.mesh.pick_poly(world_mouse_pos) };
						copyOp.pid = pid;
						copyOp.pending = true;
					}
					else
					{
						copyOp.pending = false;
					}
				}
			}
			break;
			// paste
			case GLFW_KEY_V:
			{
				if (!_modifiers)
				{
					if (copyOp.pending)
					{
						cinolib::vec3d world_mouse_pos;
						if (gui.unproject(mouse.position, world_mouse_pos))
						{
							const unsigned int pid{ grid.mesh.pick_poly(world_mouse_pos) };
							if (grid.merge(copyOp.pid, pid))
							{
								copyOp.pending = false;
								refit();
							}
							else
							{
								std::cout << "Merge failed" << std::endl;
							}
						}
					}
				}
			}
			break;
			// refine hexahedron
			case GLFW_KEY_H:
			{
				if (!_modifiers)
				{
					cinolib::vec3d world_mouse_pos;
					if (gui.unproject(mouse.position, world_mouse_pos))
					{
						const unsigned int pid{ grid.mesh.pick_poly(world_mouse_pos) };
						grid.add_refine(pid);
					}
				}
			}
			break;
			// refine face
			case GLFW_KEY_F:
			{
				if (!_modifiers)
				{
					cinolib::vec3d world_mouse_pos;
					if (gui.unproject(mouse.position, world_mouse_pos))
					{
						for (unsigned int fid_plus_one{ grid.mesh.num_faces() }; fid_plus_one > 0; fid_plus_one--)
						{
							const unsigned int fid{ fid_plus_one - 1 };
							if (!grid.mesh.adj_f2p(fid).size())
							{
								grid.mesh.face_remove_unreferenced(fid);
							}
						}
						const unsigned int fid{ grid.mesh.pick_face(world_mouse_pos) };
						grid.add_face_refine(fid);
					}
				}
			}
			break;
			// delete hexahedron
			case GLFW_KEY_DELETE:
			{
				if (!_modifiers)
				{
					cinolib::vec3d world_mouse_pos;
					if (gui.unproject(mouse.position, world_mouse_pos))
					{
						const unsigned int pid{ grid.mesh.pick_poly(world_mouse_pos) };
						grid.add_remove(pid);
						refit();
					}
				}
			}
			break;
			// make conformant
			case GLFW_KEY_Q:
			{
				if (!_modifiers)
				{
					grid.make_conforming();
				}
			}
			break;
			// save tree
			case GLFW_KEY_S:
			{
				// save tree
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					const std::string filename{ cinolib::file_dialog_save() };
					if (!filename.empty())
					{
						grid.op_tree.serialize(filename);
					}
				}
				// save mesh
				else if (_modifiers == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT))
				{
					const std::string filename{ cinolib::file_dialog_save() };
					if (!filename.empty())
					{
						grid.save_as_mesh(filename);
					}
				}
			}
			break;
			// load tree
			case GLFW_KEY_O:
			{
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					const std::string filename{ cinolib::file_dialog_open() };
					if (!filename.empty())
					{
						grid.clear();
						moveOp.pending = copyOp.pending = false;
						grid.op_tree.deserialize(filename);
						grid.apply_tree(grid.op_tree.root->operations, 0);
						refit();
					}
				}
			}
			break;
			// toggle target visibility
			case GLFW_KEY_T:
			{
				if (!_modifiers)
				{
					if (!target.p_mesh)
					{
						target.filename = cinolib::file_dialog_open();
						if (!target.filename.empty())
						{
							target.p_mesh = new cinolib::DrawableTrimesh<>(target.filename.c_str());
							gui.push(target.p_mesh);
						}
					}
					else
					{
						gui.pop(target.p_mesh);
						target.filename = "";
						delete target.p_mesh;
						target.p_mesh = nullptr;
					}
					refit();
				}
			}
			break;
			// project to target
			case GLFW_KEY_P:
			{
				if (!_modifiers)
				{
					grid.project_on_target(*target.p_mesh);
					refit();
				}
			}
			break;
			// undo or redo
			case GLFW_KEY_Z:
			{
				// undo
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					grid.undo();
					refit();
				}
				// redo
				else if (_modifiers == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT))
				{
					grid.redo();
					refit();
				}
			}
			break;
			// clear
			case GLFW_KEY_N:
			{
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					grid.clear();
					moveOp.pending = copyOp.pending = false;
					refit(false);
				}
			}
			break;
		}
	};

	gui.callback_app_controls = [&]() {
		if (moveOp.pending)
		{
			ImGui::Text("Moving vertex #%u", moveOp.vid);
		}
		if (copyOp.pending)
		{
			ImGui::Text("Copying hexahedron #%u", copyOp.pid);
		}
		if (target.p_mesh)
		{
			ImGui::Text("Showing target '%s'", target.filename.c_str());
		}
	};

	gui.launch();
	return EXIT_SUCCESS;
}

