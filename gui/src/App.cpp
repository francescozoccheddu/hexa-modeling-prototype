#pragma once

#include <HMP/Gui/App.hpp>
#include <cinolib/gl/volume_mesh_controls.h>
#include <cinolib/gl/glcanvas.h>
#include <cstdlib>
#include <iostream>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cinolib/gl/file_dialog_open.h>
#include <cinolib/gl/file_dialog_save.h>
#include <HMP/Gui/Dag/createLayout.hpp>
#include <utility>

namespace HMP::Gui
{

	// Actions

	void App::updateHighlight()
	{
		unsigned int pid{};
		cinolib::vec3d world;
		m_canvas.pop_all_markers();
		const bool pending{ m_canvas.unproject(m_mouse.position, world) };
		if (pending)
		{
			pid = m_grid.mesh.pick_poly(world);
			const unsigned int fid{ m_grid.mesh.pick_face(world) };
			const unsigned int vid{ m_grid.mesh.pick_vert(world) };
			m_canvas.push_marker(m_grid.mesh.face_centroid(fid), "", c_highlightFaceColor, c_highlightFaceRadius);
			m_canvas.push_marker(m_grid.mesh.vert(vid), "", c_highlightVertexColor, c_highlightVertexRadius);
		}
		if (pending != m_highlight.pending || (pending && pid != m_highlight.pid))
		{
			if (m_highlight.pending && m_highlight.pid < m_grid.mesh.num_polys())
			{
				m_grid.mesh.poly_data(m_highlight.pid).color = cinolib::Color::WHITE();
			}
			if (pending)
			{
				m_grid.mesh.poly_data(pid).color = c_highlightPolyColor;
			}
			m_grid.mesh.updateGL();
		}
		m_highlight.pid = pid;
		m_highlight.pending = pending;
		m_dagViewer.highlight = pending ? m_grid.mesh.poly_data(pid).element : nullptr;
	}

	void App::updateDagViewer()
	{
		m_dagViewer.layout = Dag::createLayout(*m_grid.op_tree.root);
		m_dagViewer.resetView();
	}

	// Events

	void App::onCameraChange()
	{
		updateHighlight();
	}

	bool App::onMouseMove(double _x, double _y)
	{
		m_mouse.position = cinolib::vec2d{ _x, _y };
		updateHighlight();
		return false;
	}

	bool App::onKeyPress(int _key, int _modifiers)
	{
		switch (_key)
		{
			// move vertex
			case GLFW_KEY_M:
			{
				if (!_modifiers)
				{
					onMove();
				}
			}
			break;
			// extrude
			case GLFW_KEY_E:
			{
				if (!_modifiers)
				{
					onExtrude();
				}
			}
			break;
			// copy
			case GLFW_KEY_C:
			{
				if (!_modifiers)
				{
					onCopy();
				}
			}
			break;
			// paste
			case GLFW_KEY_V:
			{
				if (!_modifiers)
				{
					onPaste();
				}
			}
			break;
			// refine hexahedron
			case GLFW_KEY_H:
			{
				if (!_modifiers)
				{
					onRefineHexahedron();
				}
			}
			break;
			// refine face
			case GLFW_KEY_F:
			{
				if (!_modifiers)
				{
					onRefineFace();
				}
			}
			break;
			// delete hexahedron
			case GLFW_KEY_DELETE:
			{
				if (!_modifiers)
				{
					onDeleteHexahedron();
				}
			}
			break;
			// make conformant
			case GLFW_KEY_Q:
			{
				if (!_modifiers)
				{
					onMakeConformant();
				}
			}
			break;
			// save tree
			case GLFW_KEY_S:
			{
				// save tree
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					onSaveTree();
				}
				// save mesh
				else if (_modifiers == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT))
				{
					onSaveMesh();
				}
			}
			break;
			// load tree
			case GLFW_KEY_O:
			{
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					onLoadTree();
				}
			}
			break;
			// toggle target visibility
			case GLFW_KEY_T:
			{
				if (!_modifiers)
				{
					onToggleTargetVisibility();
				}
			}
			break;
			// project to target
			case GLFW_KEY_P:
			{
				if (!_modifiers)
				{
					onProjectToTarget();
				}
			}
			break;
			// undo or redo
			case GLFW_KEY_Z:
			{
				// undo
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					onUndo();
				}
				// redo
				else if (_modifiers == (GLFW_MOD_CONTROL | GLFW_MOD_SHIFT))
				{
					onRedo();
				}
			}
			break;
			// clear
			case GLFW_KEY_N:
			{
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					onClear();
				}
			}
			break;
		}
		return false;
	}

	void App::onDrawControls()
	{
		m_dagViewer.draw();
		if (m_move.pending)
		{
			ImGui::Text("Moving vertex #%u", m_move.vid);
		}
		if (m_copy.pending)
		{
			ImGui::Text("Copying hexahedron #%u", m_copy.pid);
		}
		if (m_target.p_mesh)
		{
			ImGui::Text("Showing target '%s'", m_target.filename.c_str());
		}
	}

	// Commands

	void App::onMove()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			if (m_move.pending)
			{
				// FIXME move in plane instead of using world_mouse_pos
				m_grid.move_vert(m_move.vid, world_mouse_pos);
				m_move.pending = false;
				m_canvas.refit_scene();
			}
			else
			{
				m_move.vid = m_grid.mesh.pick_vert(world_mouse_pos);
				m_move.pending = true;
			}
		}
		else
		{
			m_move.pending = false;
		}
	}

	void App::onExtrude()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const unsigned int pid{ m_grid.mesh.pick_poly(world_mouse_pos) };
			unsigned int closest_fid{};
			{
				double closest_dist{ std::numeric_limits<double>::infinity() };
				for (const unsigned int fid : m_grid.mesh.poly_faces_id(pid))
				{
					const double dist{ world_mouse_pos.dist_sqrd(m_grid.mesh.face_centroid(fid)) };
					if (dist < closest_dist)
					{
						closest_dist = dist;
						closest_fid = fid;
					}
				}
			};
			const unsigned int offset{ m_grid.mesh.poly_face_offset(pid, closest_fid) };
			m_grid.add_extrude(pid, offset);
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onCopy()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const unsigned int pid{ m_grid.mesh.pick_poly(world_mouse_pos) };
			m_copy.pid = pid;
			m_copy.pending = true;
		}
		else
		{
			m_copy.pending = false;
		}
	}

	void App::onPaste()
	{
		if (m_copy.pending)
		{
			cinolib::vec3d world_mouse_pos;
			if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
			{
				const unsigned int pid{ m_grid.mesh.pick_poly(world_mouse_pos) };
				if (m_grid.merge(m_copy.pid, pid))
				{
					m_copy.pending = false;
					updateDagViewer();
					m_canvas.refit_scene();
				}
				else
				{
					std::cout << "Merge failed" << std::endl;
				}
			}
		}
	}

	void App::onRefineHexahedron()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const unsigned int pid{ m_grid.mesh.pick_poly(world_mouse_pos) };
			m_grid.add_refine(pid);
			updateDagViewer();
			updateHighlight();
		}
	}

	void App::onDeleteHexahedron()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const unsigned int pid{ m_grid.mesh.pick_poly(world_mouse_pos) };
			m_grid.add_remove(pid);
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onRefineFace()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			for (unsigned int fid_plus_one{ m_grid.mesh.num_faces() }; fid_plus_one > 0; fid_plus_one--)
			{
				const unsigned int fid{ fid_plus_one - 1 };
				if (!m_grid.mesh.adj_f2p(fid).size())
				{
					m_grid.mesh.face_remove_unreferenced(fid);
				}
			}
			const unsigned int fid{ m_grid.mesh.pick_face(world_mouse_pos) };
			m_grid.add_face_refine(fid);
			updateDagViewer();
			updateHighlight();
		}
	}

	void App::onMakeConformant()
	{
		m_grid.make_conforming();
		updateDagViewer();
		updateHighlight();
	}

	void App::onSaveMesh()
	{
		const std::string filename{ cinolib::file_dialog_save() };
		if (!filename.empty())
		{
			m_grid.save_as_mesh(filename);
		}
	}

	void App::onSaveTree()
	{
		const std::string filename{ cinolib::file_dialog_save() };
		if (!filename.empty())
		{
			m_grid.op_tree.serialize(filename);
		}
	}

	void App::onLoadTree()
	{
		const std::string filename{ cinolib::file_dialog_open() };
		if (!filename.empty())
		{
			m_grid.clear();
			m_move.pending = m_copy.pending = false;
			m_grid.op_tree.deserialize(filename);
			m_grid.apply_tree(m_grid.op_tree.root->children().vector(), 0);
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onToggleTargetVisibility()
	{
		if (!m_target.p_mesh)
		{
			m_target.filename = cinolib::file_dialog_open();
			if (!m_target.filename.empty())
			{
				m_target.p_mesh = new cinolib::DrawableTrimesh<>(m_target.filename.c_str());
				m_canvas.push(m_target.p_mesh);
			}
		}
		else
		{
			m_canvas.pop(m_target.p_mesh);
			m_target.filename = "";
			delete m_target.p_mesh;
			m_target.p_mesh = nullptr;
		}
		m_canvas.refit_scene();
	}

	void App::onProjectToTarget()
	{
		m_grid.project_on_target(*m_target.p_mesh);
		updateDagViewer();
		m_canvas.refit_scene();
	}

	void App::onUndo()
	{
		m_grid.undo();
		updateDagViewer();
		m_canvas.refit_scene();
	}

	void App::onRedo()
	{
		m_grid.redo();
		updateDagViewer();
		m_canvas.refit_scene();
	}

	void App::onClear()
	{
		m_grid.clear();
		m_move.pending = m_copy.pending = false;
		updateDagViewer();
		m_canvas.refit_scene();
	}

	// Launch

	int App::launch()
	{
		cinolib::VolumeMeshControls<HMP::MeshGrid> menu{ &m_grid.mesh, &m_canvas, "Grid mesh" };
		m_canvas.push(&m_grid.mesh);
		m_canvas.push(&menu);
		m_grid.mesh.show_mesh_flat();

		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMove(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPress(_args...); };
		m_canvas.callback_app_controls = [this](auto && ..._args) { return onDrawControls(_args ...); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChange(_args...); };

		updateDagViewer();

		return m_canvas.launch();
	}

}