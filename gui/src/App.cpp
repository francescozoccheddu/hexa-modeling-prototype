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
#include <HMP/Actions/Rotate.hpp>
#include <HMP/Actions/Paste.hpp>
#include <HMP/Utils/Serialization.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/HrDescriptions.hpp>
#include <sstream>

namespace HMP::Gui
{

	const cinolib::Color backgroundColor{ cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.1f) };

	// Actions

	void App::updateMarkers()
	{
		static const cinolib::Color overlayColor{ cinolib::Color::hsv2rgb(0.1f, 0.75f, 0.9f) };
		static const cinolib::Color mutedOverlayColor{ cinolib::Color::hsv2rgb(0.1f, 0.0f, 1.0f, 0.3f) };
		constexpr float highlightRadius{ 4.0f };
		constexpr float nameFontSize{ 20.0f };
		m_canvas.pop_all_markers();
		if (m_mouse.element)
		{
			const Id pid{ m_mesher.elementToPid(*m_mouse.element) };
			const Id vid{ m_mesh.poly_vert_id(pid, m_mouse.vertOffset) };
			const Id forwardFid{ m_mesh.poly_face_id(pid, m_mouse.faceOffset) };
			const Id upFid{ m_mesh.poly_face_id(pid, m_mouse.upFaceOffset) };
			const Id eid{ m_mesh.face_shared_edge(forwardFid, upFid) };
			m_canvas.push_marker(Meshing::Utils::midpoint(m_mesh, eid), "", overlayColor, highlightRadius);
			m_canvas.push_marker(m_mesh.vert(m_mesh.poly_vert_id(pid, m_mouse.vertOffset)), "", overlayColor, highlightRadius);
		}
		if (m_options.showNames)
		{
			for (const HMP::Dag::Node* node : HMP::Dag::Utils::descendants(*m_project.root()))
			{
				if (node->isElement())
				{
					const Id pid{ m_mesher.elementToPid(node->element()) };
					if (pid != noId)
					{
						cinolib::Color color{ m_mouse.element == node ? overlayColor : mutedOverlayColor };
						if (m_mouse.element && m_mouse.element != node)
						{
							color.a /= 5;
						}
						m_canvas.push_marker(m_mesh.poly_centroid(pid), m_dagNamer(node), color, 0, nameFontSize);
					}
				}
			}
		}
	}

	void App::updateMouse()
	{
		HMP::Dag::Element* const lastElement{ m_mouse.element };
		const Id lastFaceOffset{ m_mouse.faceOffset }, lastUpFaceOffset{ m_mouse.upFaceOffset }, lastVertOffset{ m_mouse.vertOffset };
		m_mouse.element = nullptr;
		m_mouse.faceOffset = m_mouse.vertOffset = noId;
		if (m_canvas.unproject(m_mouse.position, m_mouse.worldPosition))
		{
			// poly
			const Id pid{ m_mesh.pick_poly(m_mouse.worldPosition) };
			m_mouse.element = &m_mesher.pidToElement(pid);
			// face
			const Id fid{ Meshing::Utils::closestPolyFid(m_mesh, pid, m_mouse.worldPosition) };
			m_mouse.faceOffset = m_mesh.poly_face_offset(pid, fid);
			// up face
			const Id eid{ Meshing::Utils::closestFaceEid(m_mesh, fid, m_mouse.worldPosition) };
			const Id upFid{ Meshing::Utils::adjacentFid(m_mesh, pid, fid, eid) };
			m_mouse.upFaceOffset = m_mesh.poly_face_offset(pid, upFid);
			// vert
			const Id vid{ Meshing::Utils::closestFaceVid(m_mesh, fid, m_mouse.worldPosition) };
			m_mouse.vertOffset = m_mesh.poly_vert_offset(pid, vid);
		}
		m_dagViewer.highlight = m_mouse.element;
		if (m_mouse.element != lastElement)
		{
			m_mesher.polyMarkerSet().clear();
			if (m_mouse.element)
			{
				m_mesher.polyMarkerSet().add(*m_mouse.element);
			}
		}
		if (m_mouse.element != lastElement || m_mouse.faceOffset != lastFaceOffset)
		{
			m_mesher.faceMarkerSet().clear();
			if (m_mouse.element)
			{
				m_mesher.faceMarkerSet().add(*m_mouse.element, m_mouse.faceOffset);
			}
			m_mesher.updateMeshMarkers();
		}
		if (m_mouse.element != lastElement || m_mouse.faceOffset != lastFaceOffset || m_mouse.upFaceOffset != lastUpFaceOffset || m_mouse.vertOffset != lastVertOffset)
		{
			updateMarkers();
		}
		updateMove();
	}

	void App::updateDagViewer()
	{
		if (m_project.root())
		{
			m_dagViewer.layout() = Dag::createLayout(*m_project.root());
		}
		m_dagViewer.resetView();
	}

	void App::updateMove()
	{
		if (m_move.element)
		{
			const Id pid{ m_mesher.elementToPid(*m_move.element) };
			const Id vid{ m_mesh.poly_vert_id(pid, m_move.vertOffset) };
			if (m_mesher.getVert(m_mouse.worldPosition) == noId)
			{
				m_mesher.moveVert(vid, m_mouse.worldPosition);
			}
			updateMarkers();
			m_mesher.updateMesh();
		}
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
				if (_modifiers == GLFW_MOD_CONTROL)
				{
					onCopy();
				}
			}
			break;
			// paste
			case GLFW_KEY_V:
			{
				if (_modifiers == GLFW_MOD_CONTROL)
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
			// rotate
			case GLFW_KEY_Y:
			{
				if (!_modifiers)
				{
					onRotate();
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
		// names
		{
			bool showNames{ m_options.showNames };
			ImGui::Checkbox("Show element names", &showNames);
			if (showNames != m_options.showNames)
			{
				m_options.showNames = showNames;
				updateMarkers();
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset names"))
			{
				m_dagNamer.reset();
				updateMarkers();
			}
		}
	}

	void App::onDrawCustomGui()
	{
		if (m_mouse.element)
		{
			std::ostringstream stream{};
			stream
				<< HrDescriptions::name(*m_mouse.element, m_dagNamer)
				<< " ("
				<< "faces " << HrDescriptions::describeFaces(m_mouse.faceOffset, m_mouse.upFaceOffset)
				<< ", vert " << m_mouse.vertOffset
				<< ")";
			ImGui::Text(stream.str().c_str());
		}
		if (m_copy.element)
		{
			std::ostringstream stream{};
			stream
				<< "Copied"
				<< " " << HrDescriptions::name(*m_copy.element, m_dagNamer);
			ImGui::Text(stream.str().c_str());
		}
		if (m_move.element)
		{
			std::ostringstream stream{};
			stream
				<< "Moving"
				<< " vert " << m_move.vertOffset
				<< " of " << HrDescriptions::name(*m_move.element, m_dagNamer)
				<< " from " << HrDescriptions::describe(m_move.startPosition)
				<< " to " << HrDescriptions::describe(m_mouse.worldPosition);
			ImGui::Text(stream.str().c_str());
		}
	}

	// Commands

	void App::onMove()
	{
		if (m_mouse.element)
		{
			if (m_move.element && m_mesher.has(*m_move.element))
			{
				const Id pid{ m_mesher.elementToPid(*m_move.element) };
				const Id vid{ m_mesh.poly_vert_id(pid, m_move.vertOffset) };
				m_mesher.moveVert(vid, m_move.startPosition);
				m_commander.apply(*new Actions::MoveVert{ *m_move.element, m_move.vertOffset, m_mouse.worldPosition });
				m_move.element = nullptr;
				m_canvas.refit_scene();
			}
			else
			{
				m_move.element = m_mouse.element;
				m_move.vertOffset = m_mouse.vertOffset;
				const Id pid{ m_mesher.elementToPid(*m_move.element) };
				const Id vid{ m_mesh.poly_vert_id(pid, m_move.vertOffset) };
				m_move.startPosition = m_mesh.vert(vid);
			}
		}
		else
		{
			const Id pid{ m_mesher.elementToPid(*m_move.element) };
			const Id vid{ m_mesh.poly_vert_id(pid, m_move.vertOffset) };
			m_mesher.moveVert(vid, m_move.startPosition);
			m_move.element = nullptr;
			m_canvas.refit_scene();
			m_mesher.updateMesh();
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
				m_commander.apply(*new Actions::Paste{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset, static_cast<HMP::Dag::Extrude&>(m_copy.element->parents().single()) });
				m_canvas.refit_scene();
				updateDagViewer();
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

	void App::onRotate()
	{
		if (m_mouse.element && m_mouse.element->parents().size() == 1 && m_mouse.element->parents().single().primitive() == HMP::Dag::Operation::EPrimitive::Extrude)
		{
			m_commander.apply(*new Actions::Rotate{ static_cast<HMP::Dag::Extrude&>(m_mouse.element->parents().single()) });
			m_mouse.element = nullptr;
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
		if (m_targetWidget.hasMesh())
		{
			m_targetWidget.visible() ^= true;
			m_targetWidget.updateVisibility();
		}
	}

	void App::onProjectToTarget()
	{
		m_commander.apply(*new Actions::Project{ m_targetWidget.mesh() });
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
			updateMarkers();
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
			updateMarkers();
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

	// launch

	App::App() :
		m_project{}, m_canvas{}, m_mesher{ m_project.mesher() }, m_mesh{ m_mesher.mesh() }, m_commander{ m_project.commander() },
		m_dagNamer{}, m_dagViewer{ m_mesher, m_dagNamer }, m_menu{ const_cast<Meshing::Mesher::Mesh*>(&m_mesh), &m_canvas, "Mesh controls" },
		m_commanderWidget{ m_commander, m_dagNamer }, m_axesWidget{ m_canvas.camera }, m_targetWidget{ m_mesh }
	{

		m_commander.apply(*new Actions::Clear());
		m_commander.applied().clear();

		m_commander.applied().limit(100);
		m_commander.unapplied().limit(100);

		m_canvas.push(&m_mesh);
		m_canvas.push(&m_axesWidget);

		m_canvas.push(&m_commanderWidget);
		m_canvas.push(&m_targetWidget);
		m_canvas.push(&m_menu);
		m_canvas.push(&m_dagViewer);

		m_targetWidget.onProjectRequest() = [this](const Widgets::Target& _target) { onProjectToTarget(); };
		m_targetWidget.onMeshLoad() = [this](const Widgets::Target& _target) { m_canvas.push(&_target.mesh(), false); };
		m_targetWidget.onMeshClear() = [this](const Widgets::Target& _target) { m_canvas.pop(&_target.mesh()); };

		m_canvas.background = backgroundColor;
		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMove(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPress(_args...); };
		m_canvas.callback_app_controls = [this](auto && ..._args) { return onDrawControls(_args ...); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChange(_args...); };
		m_canvas.callback_custom_gui = [this](auto && ..._args) { return onDrawCustomGui(_args...); };

		updateDagViewer();
	}

	int App::launch()
	{
		updateDagViewer();
		return m_canvas.launch();
	}

}