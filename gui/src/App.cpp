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
#include <HMP/Actions/Transform.hpp>
#include <HMP/Utils/Serialization.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/HrDescriptions.hpp>
#include <sstream>
#include <iomanip>

namespace HMP::Gui
{

	// Actions

	void App::updateMouseMarkers()
	{
		std::vector<cinolib::Marker>& set{ m_canvas.marker_sets[c_mouseMarkerSetInd] };
		static constexpr unsigned int highlightRadius{ 4 };
		if (m_mouse.element)
		{
			set.resize(2);
			const Id pid{ m_mesher.elementToPid(*m_mouse.element) };
			const Id vid{ m_mesh.poly_vert_id(pid, m_mouse.vertOffset) };
			const Id forwardFid{ m_mesh.poly_face_id(pid, m_mouse.faceOffset) };
			const Id upFid{ m_mesh.poly_face_id(pid, m_mouse.upFaceOffset) };
			const Id eid{ m_mesh.face_shared_edge(forwardFid, upFid) };
			set[0] = cinolib::Marker{
					.pos_3d{m_mesh.vert(m_mesh.poly_vert_id(pid, m_mouse.vertOffset))},
					.color{c_overlayColor},
					.shape_radius{highlightRadius},
					.shape{cinolib::Marker::EShape::CircleFilled}
			};
			set[1] = cinolib::Marker{
					.pos_3d{Meshing::Utils::midpoint(m_mesh, eid)},
					.color{c_overlayColor},
					.shape_radius{highlightRadius},
					.shape{cinolib::Marker::EShape::Cross45},
					.line_thickness{2.0f}
			};
		}
		else
		{
			set.clear();
		}
	}

	void App::updateVertSelectionMarkers()
	{}

	void App::updateElementsMarkers()
	{
		std::vector<cinolib::Marker>& set{ m_canvas.marker_sets[c_elementsMarkerSetInd] };
		set.clear();
		static constexpr unsigned int nameFontSize{ 20 };
		if (m_options.showNames)
		{
			set.reserve(m_mesh.num_polys());
			for (std::size_t pid{}; pid < m_mesh.num_polys(); pid++)
			{
				if (m_mesh.poly_is_on_surf(pid))
				{
					const HMP::Dag::Element& element{ m_mesher.pidToElement(pid) };
					cinolib::Color color{ m_mouse.element == &element ? c_overlayColor : c_mutedOverlayColor };
					if (m_mouse.element && m_mouse.element != &element)
					{
						color.a() /= 5;
					}
					set.push_back(cinolib::Marker{
						.pos_3d{m_mesh.poly_centroid(pid)},
						.text{m_dagNamer(&element)},
						.color{color},
						.shape_radius{0},
						.font_size{nameFontSize}
						});
				}
			}
		}
	}

	void App::updateAllMarkers()
	{
		updateMouseMarkers();
		updateElementsMarkers();
		updateVertSelectionMarkers();
	}

	void App::updateMouse()
	{
		HMP::Dag::Element* const lastElement{ m_mouse.element };
		const Id lastFaceOffset{ m_mouse.faceOffset }, lastUpFaceOffset{ m_mouse.upFaceOffset }, lastVertOffset{ m_mouse.vertOffset };
		m_mouse.element = nullptr;
		m_mouse.faceOffset = m_mouse.vertOffset = noId;
		{
			// poly
			Id pid, fid, eid, vid;
			const cinolib::Ray ray{ m_canvas.eye_to_mouse_ray() };
			if (m_mesher.pick(ray.begin(), ray.dir(), pid, fid, eid, vid))
			{
				m_mouse.element = &m_mesher.pidToElement(pid);
				m_mouse.faceOffset = m_mesh.poly_face_offset(pid, fid);
				const Id upFid{ Meshing::Utils::adjacentFid(m_mesh, pid, fid, eid) };
				m_mouse.upFaceOffset = m_mesh.poly_face_offset(pid, upFid);
				m_mouse.vertOffset = m_mesh.poly_vert_offset(pid, vid);
			}
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
			updateMouseMarkers();
			updateElementsMarkers();
		}
		updateMove();
	}

	void App::updateDagViewer()
	{
		m_dagViewerNeedsUpdate = true;
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
			updateAllMarkers();
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
					onRefineElement(false);
				}
				else if (_modifiers == GLFW_MOD_SHIFT)
				{
					onRefineElement(true);
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
			// print elements
			case GLFW_KEY_COMMA:
			{
				std::cout << "-------- PRINT DEBUG INFO --------\n";
				std::cout << "---- Elements\n";
				std::vector<Meshing::Utils::PolyVertLoc> locs{};
				locs.reserve(8);
				for (const auto [element, pid] : m_mesher)
				{
					locs.clear();
					const HMP::Vec centroid{ m_mesh.poly_centroid(pid) };
					for (const Id vid : m_mesh.adj_p2v(pid))
					{
						locs.push_back(Meshing::Utils::polyVertLoc(m_mesh.vert(vid), centroid));
					}
					std::cout
						<< "name: " << m_dagNamer(&element)
						<< " pid: " << pid
						<< " centroid: " << HrDescriptions::describe(centroid)
						<< " vids: " << HrDescriptions::describe(m_mesh.adj_p2v(pid))
						<< " eids: " << HrDescriptions::describe(m_mesh.adj_p2e(pid))
						<< " fids: " << HrDescriptions::describe(m_mesh.adj_p2f(pid))
						<< " pids: " << HrDescriptions::describe(m_mesh.adj_p2p(pid))
						<< " winding: " << HrDescriptions::describe(m_mesh.poly_faces_winding(pid))
						<< " locs: " << HrDescriptions::describe(locs)
						<< "\n";
				}
				std::cout << "---- Faces\n";
				for (std::size_t fid{}; fid < m_mesh.num_faces(); fid++)
				{
					std::cout
						<< "fid: " << fid
						<< " centroid: " << HrDescriptions::describe(m_mesh.face_centroid(fid))
						<< " vids: " << HrDescriptions::describe(m_mesh.adj_f2v(fid))
						<< " eids: " << HrDescriptions::describe(m_mesh.adj_f2e(fid))
						<< " fids: " << HrDescriptions::describe(m_mesh.adj_f2f(fid))
						<< " pids: " << HrDescriptions::describe(m_mesh.adj_f2p(fid))
						<< " normal: " << HrDescriptions::describe(m_mesh.face_data(fid).normal)
						<< "\n";
				}
				std::cout << "---- Edges\n";
				for (std::size_t eid{}; eid < m_mesh.num_edges(); eid++)
				{
					std::cout
						<< "eid: " << eid
						<< " midpoint: " << HrDescriptions::describe(Meshing::Utils::midpoint(m_mesh, eid))
						<< " vids: " << HrDescriptions::describe(m_mesh.adj_e2v(eid))
						<< " eids: " << HrDescriptions::describe(m_mesh.adj_e2e(eid))
						<< " fids: " << HrDescriptions::describe(m_mesh.adj_e2f(eid))
						<< " pids: " << HrDescriptions::describe(m_mesh.adj_e2p(eid))
						<< "\n";
				}
				std::cout << "---- Vertices\n";
				for (std::size_t vid{}; vid < m_mesh.num_verts(); vid++)
				{
					std::cout
						<< "vid: " << vid
						<< " position: " << HrDescriptions::describe(m_mesh.vert(vid))
						<< " vids: " << HrDescriptions::describe(m_mesh.adj_v2v(vid))
						<< " eids: " << HrDescriptions::describe(m_mesh.adj_v2e(vid))
						<< " fids: " << HrDescriptions::describe(m_mesh.adj_v2f(vid))
						<< " pids: " << HrDescriptions::describe(m_mesh.adj_v2p(vid))
						<< "\n";
				}
				std::cout << "----------------------------------" << std::endl;
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
				updateElementsMarkers();
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset names"))
			{
				m_dagNamer.reset();
				updateElementsMarkers();
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
			ImGui::Text("%s", stream.str().c_str());
		}
		if (m_copy.element)
		{
			std::ostringstream stream{};
			stream
				<< "Copied"
				<< " " << HrDescriptions::name(*m_copy.element, m_dagNamer);
			ImGui::Text("%s", stream.str().c_str());
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
			ImGui::Text("%s", stream.str().c_str());
		}
	}

	void App::onDagViewerDraw()
	{
		if (m_dagViewerNeedsUpdate)
		{
			m_dagViewerNeedsUpdate = false;
			if (m_project.root())
			{
				m_dagViewer.layout() = Dag::createLayout(*m_project.root());
			}
			m_dagViewer.resetView();
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
		else if (m_move.element)
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

	void App::onRefineElement(bool _twice)
	{
		if (m_mouse.element)
		{
			m_commander.apply(*new Actions::Refine{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset, Meshing::ERefinementScheme::Subdivide3x3, _twice ? 2u : 1u });
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
		cinolib::DrawableTrimesh<> target{ m_targetWidget.mesh() };
		for (Id vid{}; vid < target.num_verts(); vid++)
		{
			Vec& vert{ target.vert(vid) };
			vert = target.transform * vert;
		}
		m_commander.apply(*new Actions::Project{ std::move(target) });
		updateDagViewer();
		m_canvas.refit_scene();
	}

	void App::onApplyTargetTransform(const Mat4& _transform)
	{
		m_commander.apply(*new Actions::Transform{ _transform });
		updateAllMarkers();
		m_canvas.reset_camera();
	}

	void App::onUndo()
	{
		if (m_commander.canUndo())
		{
			m_commander.undo();
			updateDagViewer();
			m_canvas.refit_scene();
			updateAllMarkers();
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
			updateAllMarkers();
		}
		else
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
		m_commanderWidget{ m_commander, m_dagNamer }, m_axesWidget{ m_canvas.camera }, m_targetWidget{ m_mesh }, m_dagViewerNeedsUpdate{ true }
	{

		m_canvas.background = c_backgroundColor;
		m_mesher.polyMarkerSet().color() = c_selectedPolyColor;
		m_mesher.faceMarkerSet().color() = c_selectedFaceColor;

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
		m_targetWidget.onApplyTransformToSource() = [this](const Widgets::Target& _target, const Mat4& _transform) { onApplyTargetTransform(_transform); };

		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMove(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPress(_args...); };
		m_canvas.callback_app_controls = [this](auto && ..._args) { return onDrawControls(_args ...); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChange(_args...); };
		m_canvas.callback_custom_gui = [this](auto && ..._args) { return onDrawCustomGui(_args...); };
		m_canvas.marker_sets.resize(3);

		m_dagViewer.onDraw += [this]() { onDagViewerDraw(); };
		updateDagViewer();
	}

	int App::launch()
	{
		return m_canvas.launch();
	}

}