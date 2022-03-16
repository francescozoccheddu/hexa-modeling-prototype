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

int main()
{
	Grid grid;
	cinolib::GLcanvas gui;
	cinolib::VolumeMeshControls<MeshGrid> menu{ &grid.mesh, &gui, "Grid mesh" };
	gui.push(&grid.mesh);
	gui.push(&menu);

	struct
	{
		cinolib::vec2d pos{};
		bool pressed{ false };
	} mouse;

	struct
	{
		unsigned int vid{};
		bool pending{ false };
	} move_op;

	struct
	{
		unsigned int pid{};
		bool pending{ false };
	} copy_op;

	struct
	{
		cinolib::DrawableTrimesh<>* p_mesh{ nullptr };
		std::string filename{};
	} target;

	gui.callback_mouse_moved = [&](double _x, double _y)
	{
		mouse.pos = cinolib::vec2d{ _x, _y };
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
					if (gui.unproject(mouse.pos, world_mouse_pos))
					{
						if (move_op.pending)
						{
							// FIXME move in plane instead of using world_mouse_pos
							const cinolib::vec3d offset{ world_mouse_pos - grid.mesh.vert(move_op.vid) };
							grid.move_vert(move_op.vid, offset);
							move_op.pending = false;
						}
						else
						{
							move_op.vid = grid.mesh.pick_vert(world_mouse_pos);
							move_op.pending = true;
						}
					}
					else
					{
						move_op.pending = false;
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
					if (gui.unproject(mouse.pos, world_mouse_pos))
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
					if (gui.unproject(mouse.pos, world_mouse_pos))
					{
						const unsigned int pid{ grid.mesh.pick_poly(world_mouse_pos) };
						copy_op.pid = pid;
						copy_op.pending = true;
					}
					else
					{
						copy_op.pending = false;
					}
				}
			}
			break;
			// paste
			case GLFW_KEY_V:
			{
				if (!_modifiers)
				{
					if (copy_op.pending)
					{
						cinolib::vec3d world_mouse_pos;
						if (gui.unproject(mouse.pos, world_mouse_pos))
						{
							const unsigned int pid{ grid.mesh.pick_poly(world_mouse_pos) };
							if (grid.merge(copy_op.pid, pid))
							{
								copy_op.pending = false;
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
					if (gui.unproject(mouse.pos, world_mouse_pos))
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
					if (gui.unproject(mouse.pos, world_mouse_pos))
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
					if (gui.unproject(mouse.pos, world_mouse_pos))
					{
						const unsigned int pid{ grid.mesh.pick_poly(world_mouse_pos) };
						grid.add_remove(pid);
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
						move_op.pending = copy_op.pending = false;
						grid.op_tree.deserialize(filename);
						grid.apply_tree(grid.op_tree.root->operations, 0);
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
				}
			}
			break;
			// project to target
			case GLFW_KEY_P:
			{
				if (!_modifiers)
				{
					grid.project_on_target(*target.p_mesh);
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
				}
				// redo
				else if (_modifiers == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT))
				{
					grid.redo();
				}
			}
			break;
			// clear
			case GLFW_KEY_N:
			{
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					grid.clear();
					move_op.pending = copy_op.pending = false;
				}
			}
			break;
		}
	};

	gui.callback_app_controls = [&]() {
		if (move_op.pending)
		{
			ImGui::Text("Moving vertex #%u", move_op.vid);
		}
		if (copy_op.pending)
		{
			ImGui::Text("Copying hexahedron #%u", copy_op.pid);
		}
		if (target.p_mesh)
		{
			ImGui::Text("Showing target '%s'", target.filename.c_str());
		}
	};

	gui.launch();
	return EXIT_SUCCESS;
}

