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
#include <HMP/Actions/Load.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Refine.hpp>

namespace HMP::Gui
{

	// Actions

	App::App()
		: m_project{}, m_canvas{}, m_mesher{ m_project.mesher() }, m_mesh{ m_mesher.mesh() }, m_commander{ m_project.commander() }, m_dagViewer{ m_mesher }
	{
		m_commander.apply(*new Actions::Clear());
		m_commander.applied().clear();

		cinolib::VolumeMeshControls<Meshing::Mesher::Mesh> menu{ const_cast<Meshing::Mesher::Mesh*>(&m_mesh), &m_canvas, "Mesh" };
		m_canvas.push(&m_mesh);
		//m_canvas.push(&menu);

		m_canvas.background = m_mesher.suggestedBackgroundColor;
		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMove(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPress(_args...); };
		m_canvas.callback_app_controls = [this](auto && ..._args) { return onDrawControls(_args ...); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChange(_args...); };

		updateDagViewer();
	}

	void App::updateHighlight()
	{
		Id pid{ noId };
		cinolib::vec3d world;
		m_canvas.pop_all_markers();
		m_mesher.faceMarkerSet().clear();
		m_mesher.polyMarkerSet().clear();
		if (m_canvas.unproject(m_mouse.position, world))
		{
			// poly
			{
				pid = m_mesh.pick_poly(world);
				m_mesher.polyMarkerSet().add(m_mesher.pidToElement(pid));
			}
			// face
			{
				const Id fid{ m_mesh.pick_face(world) };
				const Id facePid{ m_mesh.adj_f2p(fid).front() };
				m_mesher.faceMarkerSet().add(m_mesher.pidToElement(facePid), m_mesh.poly_face_offset(facePid, fid));
			}
			// vert
			{
				const Id vid{ m_mesh.pick_vert(world) };
				m_canvas.push_marker(m_mesh.vert(vid), "", m_mesher.suggestedOverlayColor, c_highlightVertexRadius);
			}
		}
		m_dagViewer.highlight = pid != noId ? &m_mesher.pidToElement(pid) : nullptr;
		m_mesher.updateMesh();
	}

	void App::updateDagViewer()
	{
		if (m_project.root())
		{
			m_dagViewer.layout = Dag::createLayout(*m_project.root());
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
				const Id pid{ m_mesh.adj_v2p(m_move.vid).front() };
				m_commander.apply(*new Actions::MoveVert{ m_mesher.pidToElement(pid), m_mesh.poly_vert_offset(pid, m_move.vid), world_mouse_pos });
				m_move.pending = false;
				m_canvas.refit_scene();
			}
			else
			{
				m_move.vid = m_mesh.pick_vert(world_mouse_pos);
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
			const Id pid{ m_mesh.pick_poly(world_mouse_pos) };
			Id closest_fid{};
			{
				double closest_dist{ std::numeric_limits<double>::infinity() };
				for (const Id fid : m_mesh.poly_faces_id(pid))
				{
					const double dist{ world_mouse_pos.dist_sqrd(m_mesh.face_centroid(fid)) };
					if (dist < closest_dist)
					{
						closest_dist = dist;
						closest_fid = fid;
					}
				}
			};
			m_commander.apply(*new Actions::Extrude{ m_mesher.pidToElement(pid), m_mesh.poly_face_offset(pid, closest_fid) });
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onCopy()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const Id pid{ m_mesh.pick_poly(world_mouse_pos) };
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
				const Id pid{ m_mesh.pick_poly(world_mouse_pos) };
				if (/*m_mesher.merge(m_copy.pid, pid)*/ false)
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
			const Id pid{ m_mesh.pick_poly(world_mouse_pos) };
			m_commander.apply(*new Actions::Refine{ m_mesher.pidToElement(pid), 0, Meshing::ERefinementScheme::Subdivide3x3 });
			updateDagViewer();
			updateHighlight();
		}
	}

	void App::onDeleteHexahedron()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const Id pid{ m_mesh.pick_poly(world_mouse_pos) };
			m_commander.apply(*new Actions::Delete{ m_mesher.pidToElement(pid) });
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onRefineFace()
	{
		cinolib::vec3d world_mouse_pos;
		if (m_canvas.unproject(m_mouse.position, world_mouse_pos))
		{
			const Id fid{ m_mesh.pick_face(world_mouse_pos) };
			const Id pid{ m_mesh.adj_f2p(fid).front() };
			m_commander.apply(*new Actions::Refine{ m_mesher.pidToElement(pid), m_mesh.poly_face_offset(pid, fid), Meshing::ERefinementScheme::Inset });
			updateDagViewer();
			updateHighlight();
		}
	}

	void App::onMakeConformant()
	{
		m_commander.apply(*new Actions::MakeConforming());
		updateDagViewer();
		updateHighlight();
	}

	void App::onSaveMesh()
	{
		const std::string filename{ cinolib::file_dialog_save() };
		if (!filename.empty())
		{
			//m_mesher.save_as_mesh(filename);
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
			Utils::Serialization::Serializer serializer{ file };
			serializer << *m_project.root();
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
			Utils::Serialization::Deserializer deserializer{ file };
			deserializer >> root;
			file.close();
			m_commander.apply(*new Actions::Load{ root->element() });
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
		///m_mesher.project_on_target(*m_target.p_mesh);
		throw std::runtime_error{ "^^^" };
		updateDagViewer();
		m_canvas.refit_scene();
	}

	void App::onUndo()
	{
		if (m_commander.canUndo())
		{
			m_commander.undo();
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onRedo()
	{
		if (m_commander.canRedo())
		{
			m_commander.redo();
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onClear()
	{
		m_commander.apply(*new Actions::Clear());
		m_move.pending = m_copy.pending = false;
		updateDagViewer();
		m_canvas.refit_scene();
	}

	// Launch

	int App::launch()
	{
		updateDagViewer();
		return m_canvas.launch();
	}

}