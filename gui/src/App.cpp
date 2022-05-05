#pragma once

#include <HMP/Gui/App.hpp>
#include <cinolib/gl/volume_mesh_controls.h>
#include <cinolib/gl/glcanvas.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <HMP/Dag/Utils.hpp>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cinolib/gl/file_dialog_open.h>
#include <cinolib/gl/file_dialog_save.h>
#include <HMP/Gui/Dag/createLayout.hpp>
#include <utility>
#include <HMP/Actions/MoveVert.hpp>
#include <HMP/Actions/Clear.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Refine.hpp>

namespace HMP::Gui
{

	const Grid& App::grid() const
	{
		return m_project.grid();
	}

	Commander& App::commander()
	{
		return m_project.commander();
	}

	const Commander& App::commander() const
	{
		return m_project.commander();
	}

	const HMP::Dag::Element& App::root() const
	{
		return *m_project.root();
	}

	// Actions

	void App::updateHighlight()
	{
		/*
		Id pid{};
		cinolib::vec3d world;
		m_canvas.pop_all_markers();
		const bool pending{ m_canvas.unproject(m_mouse.position, world) };
		if (pending)
		{
			pid = mesh.pick_poly(world);
			const Id fid{ mesh.pick_face(world) };
			const Id vid{ mesh.pick_vert(world) };
			m_canvas.push_marker(mesh.face_centroid(fid), "", c_highlightFaceColor, c_highlightFaceRadius);
			m_canvas.push_marker(mesh.vert(vid), "", c_highlightVertexColor, c_highlightVertexRadius);
		}
		if (pending != m_highlight.pending || (pending && pid != m_highlight.pid))
		{
			if (m_highlight.pending && m_highlight.pid < mesh.num_polys())
			{
				mesh.poly_data(m_highlight.pid).color = cinolib::Color::WHITE();
			}
			if (pending)
			{
				mesh.poly_data(pid).color = c_highlightPolyColor;
			}
			mesh.updateGL();
		}
		m_highlight.pid = pid;
		m_highlight.pending = pending;
		m_dagViewer.highlight = pending ? mesh.poly_data(pid).element : nullptr;
		*/
	}

	void App::updateDagViewer()
	{
		if (std::as_const(m_project).root())
		{
			m_dagViewer.layout = Dag::createLayout(root());
		}
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
		const Project& project{ m_project };
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			if (m_move.pending)
			{
				// FIXME move in plane instead of using world_mouse_pos
				commander().apply(*new Actions::MoveVert{ grid().mesh().vert(m_move.vid), world_mouse_pos });
				m_move.pending = false;
				m_canvas.refit_scene();
			}
			else
			{
				m_move.vid = grid().mesh().pick_vert(world_mouse_pos);
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
			const Id pid{ grid().mesh().pick_poly(world_mouse_pos) };
			Id closest_fid{};
			{
				double closest_dist{ std::numeric_limits<double>::infinity() };
				for (const Id fid : grid().mesh().poly_faces_id(pid))
				{
					const double dist{ world_mouse_pos.dist_sqrd(grid().mesh().face_centroid(fid)) };
					if (dist < closest_dist)
					{
						closest_dist = dist;
						closest_fid = fid;
					}
				}
			};
			commander().apply(*new Actions::Extrude{ grid().mesh().poly_centroid(pid), grid().mesh().face_centroid(closest_fid) });
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onCopy()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const Id pid{ grid().mesh().pick_poly(world_mouse_pos) };
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
				const Id pid{ grid().mesh().pick_poly(world_mouse_pos) };
				if (/*grid().merge(m_copy.pid, pid)*/ false)
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
			const Id pid{ grid().mesh().pick_poly(world_mouse_pos) };
			commander().apply(*new Actions::Refine{ grid().mesh().poly_centroid(pid), grid().mesh().face_centroid(grid().mesh().poly_face_id(pid, 0)), Meshing::ERefinementScheme::Subdivide3x3 });
			updateDagViewer();
			updateHighlight();
		}
	}

	void App::onDeleteHexahedron()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const Id pid{ grid().mesh().pick_poly(world_mouse_pos) };
			commander().apply(*new Actions::Delete{ grid().mesh().poly_centroid(pid) });
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onRefineFace()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const Id fid{ grid().mesh().pick_face(world_mouse_pos) };
			commander().apply(*new Actions::Refine{ grid().mesh().poly_centroid(grid().mesh().adj_f2p(fid).front()), grid().mesh().face_centroid(fid), Meshing::ERefinementScheme::Subdivide3x3 });
			updateDagViewer();
			updateHighlight();
		}
	}

	void App::onMakeConformant()
	{
		commander().apply(*new Actions::MakeConforming());
		updateDagViewer();
		updateHighlight();
	}

	void App::onSaveMesh()
	{
		const std::string filename{ cinolib::file_dialog_save() };
		if (!filename.empty())
		{
			//grid().save_as_mesh(filename);
			throw std::runtime_error{ "^^^" };
		}
	}

	void App::onSaveTree()
	{
		const std::string filename{ cinolib::file_dialog_save() };
		if (!filename.empty())
		{
			using HMP::Dag::Utils::operator<<;
			std::ofstream file;
			file.open(filename);
			file << root();
			file.close();
		}
	}

	void App::onLoadTree()
	{
		const std::string filename{ cinolib::file_dialog_open() };
		if (!filename.empty())
		{
			using HMP::Dag::Utils::operator>>;
			std::ifstream file;
			file.open(filename);
			HMP::Dag::Node* root;
			file >> root;
			file.close();
			//grid().replaceDag(root->element()); 
			throw std::runtime_error{ "^^^" };
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
		///grid().project_on_target(*m_target.p_mesh);
		throw std::runtime_error{ "^^^" };
		updateDagViewer();
		m_canvas.refit_scene();
	}

	void App::onUndo()
	{
		commander().undo();
		updateDagViewer();
		m_canvas.refit_scene();
	}

	void App::onRedo()
	{
		commander().redo();
		updateDagViewer();
		m_canvas.refit_scene();
	}

	void App::onClear()
	{
		commander().apply(*new Actions::Clear());
		m_move.pending = m_copy.pending = false;
		updateDagViewer();
		m_canvas.refit_scene();
	}

	// Launch

	int App::launch()
	{
		cinolib::VolumeMeshControls<Grid::Mesh> menu{ const_cast<Grid::Mesh*>(& grid().mesh()), &m_canvas, "Grid mesh"}; // FIXME
		m_canvas.push(&grid().mesh());
		m_canvas.push(&menu);

		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMove(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPress(_args...); };
		m_canvas.callback_app_controls = [this](auto && ..._args) { return onDrawControls(_args ...); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChange(_args...); };

		updateDagViewer();

		return m_canvas.launch();
	}

}