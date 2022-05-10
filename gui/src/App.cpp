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
#include <HMP/Dag/Operation.hpp>
#include <HMP/Actions/MoveVert.hpp>
#include <HMP/Actions/Clear.hpp>
#include <HMP/Actions/Load.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Project.hpp>
#include <HMP/Actions/Refine.hpp>
#include <HMP/Actions/Paste.hpp>
#include <HMP/Utils/Serialization.hpp>
#include <HMP/Meshing/Utils.hpp>

namespace HMP::Gui
{

	// Actions

	App::App()
		: m_project{}, m_canvas{}, m_mesher{ m_project.mesher() }, m_mesh{ m_mesher.mesh() }, m_commander{ m_project.commander() }, m_dagViewer{ m_mesher }, m_menu{ const_cast<Meshing::Mesher::Mesh*>(&m_mesh), &m_canvas, "Mesh controls" }
	{
		m_commander.apply(*new Actions::Clear());
		m_commander.applied().clear();

		m_canvas.push(&m_mesh);
		m_canvas.push(&m_menu);
		m_canvas.push(&m_dagViewer);

		m_canvas.background = m_mesher.suggestedBackgroundColor;
		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMove(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPress(_args...); };
		m_canvas.callback_app_controls = [this](auto && ..._args) { return onDrawControls(_args ...); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChange(_args...); };

		updateDagViewer();
	}

	void App::updateMouse()
	{
		constexpr float highlightVertexRadius{ 4.0f };
		m_canvas.pop_all_markers();
		m_mesher.faceMarkerSet().clear();
		m_mesher.polyMarkerSet().clear();
		m_mouse.element = nullptr;
		m_mouse.faceOffset = m_mouse.vertOffset = noId;
		if (m_canvas.unproject(m_mouse.position, m_mouse.worldPosition))
		{
			// poly
			const Id pid{ m_mesh.pick_poly(m_mouse.worldPosition) };
			m_mouse.element = &m_mesher.pidToElement(pid);
			m_mesher.polyMarkerSet().add(*m_mouse.element);
			// face
			const Id fid{ Meshing::Utils::closestPolyFid(m_mesh, pid, m_mouse.worldPosition) };
			m_mouse.faceOffset = m_mesh.poly_face_offset(pid, fid);
			m_mesher.faceMarkerSet().add(*m_mouse.element, m_mouse.faceOffset);
			// up face
			const Id eid{ Meshing::Utils::closestFaceEid(m_mesh, fid, m_mouse.worldPosition) };
			const Id upFid{ Meshing::Utils::adjacentFid(m_mesh, pid, fid, eid) };
			m_mouse.upFaceOffset = m_mesh.poly_face_offset(pid, upFid);
			m_canvas.push_marker(Meshing::Utils::midpoint(m_mesh, eid), "", m_mesher.suggestedOverlayColor, highlightVertexRadius);
			// vert
			const Id vid{ Meshing::Utils::closestFaceVid(m_mesh, fid, m_mouse.worldPosition) };
			m_mouse.vertOffset = m_mesh.poly_vert_offset(pid, vid);
			m_canvas.push_marker(m_mesh.vert(m_mesh.poly_vert_id(pid, m_mouse.vertOffset)), "", m_mesher.suggestedOverlayColor, highlightVertexRadius);
		}
		m_dagViewer.highlight = m_mouse.element;
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
		updateMouse();
	}

	bool App::onMouseMove(double _x, double _y)
	{
		m_mouse.position = cinolib::vec2d{ _x, _y };
		updateMouse();
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
					onRefineElement();
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
					onDelete();
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
	{}

	// Commands

	void App::onMove()
	{
		if (m_mouse.element)
		{
			if (m_move.element && m_mesher.has(*m_move.element))
			{
				// FIXME move in plane instead of using world_mouse_pos
				m_commander.apply(*new Actions::MoveVert{ *m_move.element, m_move.vertOffset, m_mouse.worldPosition });
				m_move.element = nullptr;
				m_canvas.refit_scene();
			}
			else
			{
				m_move.element = m_mouse.element;
				m_move.vertOffset = m_mouse.vertOffset;
			}
		}
		else
		{
			m_move.element = nullptr;
		}
	}

	void App::onExtrude()
	{
		if (m_mouse.element)
		{
			m_commander.apply(*new Actions::Extrude{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset });
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onCopy()
	{
		if (m_mouse.element)
		{
			m_copy.element = m_mouse.element;
		}
		else
		{
			m_copy.element = nullptr;
		}
	}

	void App::onPaste()
	{
		if (m_mouse.element && m_copy.element && m_mesher.has(*m_copy.element))
		{
			if (m_copy.element->parents().size() == 1 && m_copy.element->parents().single().primitive() == HMP::Dag::Operation::EPrimitive::Extrude)
			{
				m_commander.apply(*new Actions::Paste{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset, static_cast<HMP::Dag::Extrude&>(m_copy.element->parents().single())});
			}
		}
	}

	void App::onRefineElement()
	{
		if (m_mouse.element)
		{
			m_commander.apply(*new Actions::Refine{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset, Meshing::ERefinementScheme::Subdivide3x3 });
			updateDagViewer();
			updateMouse();
		}
	}

	void App::onDelete()
	{
		if (m_mesh.num_polys() <= 1)
		{
			std::cout << "cannot delete the only element" << std::endl;
			return;
		}
		if (m_mouse.element)
		{
			m_commander.apply(*new Actions::Delete{ *m_mouse.element });
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onRefineFace()
	{
		if (m_mouse.element)
		{
			m_commander.apply(*new Actions::Refine{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset, Meshing::ERefinementScheme::Inset });
			updateDagViewer();
			updateMouse();
		}
	}

	void App::onMakeConformant()
	{
		m_commander.apply(*new Actions::MakeConforming());
		updateDagViewer();
		updateMouse();
	}

	void App::onSaveMesh()
	{
		const std::string filename{ cinolib::file_dialog_save() };
		if (!filename.empty())
		{
			m_mesh.save(filename.c_str());
		}
	}

	void App::onSaveTree()
	{
		const std::string filename{ cinolib::file_dialog_save() };
		if (!filename.empty())
		{
			std::ofstream file;
			file.open(filename);
			Utils::Serialization::Serializer serializer{ file };
			HMP::Dag::Utils::serialize(serializer, *m_project.root());
			file.close();
		}
	}

	void App::onLoadTree()
	{
		const std::string filename{ cinolib::file_dialog_open() };
		if (!filename.empty())
		{
			std::ifstream file;
			file.open(filename);
			Utils::Serialization::Deserializer deserializer{ file };
			HMP::Dag::Element& root = HMP::Dag::Utils::deserialize(deserializer).element();
			file.close();
			m_commander.apply(*new Actions::Load{ root });
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onToggleTargetVisibility()
	{
		if (!m_target.mesh)
		{
			m_target.filename = cinolib::file_dialog_open();
			if (!m_target.filename.empty())
			{
				m_target.mesh = new cinolib::DrawableTrimesh<>(m_target.filename.c_str());
				m_canvas.push(m_target.mesh);
			}
		}
		else
		{
			m_canvas.pop(m_target.mesh);
			m_target.filename = "";
			delete m_target.mesh;
			m_target.mesh = nullptr;
		}
		m_canvas.refit_scene();
	}

	void App::onProjectToTarget()
	{
		if (m_target.mesh)
		{
			m_commander.apply(*new Actions::Project{ *m_target.mesh });
			updateDagViewer();
			m_canvas.refit_scene();
		}
	}

	void App::onUndo()
	{
		if (m_commander.canUndo())
		{
			m_commander.undo();
			updateDagViewer();
			m_canvas.refit_scene();
		}
		else
		{
			std::cout << "cannot undo" << std::endl;
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
		{
			std::cout << "cannot redo" << std::endl;
		}
	}

	void App::onClear()
	{
		m_commander.apply(*new Actions::Clear());
		m_move.element = nullptr;
		m_copy.element = nullptr;
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