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
#include <HMP/Actions/Clear.hpp>
#include <HMP/Actions/Load.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Project.hpp>
#include <HMP/Actions/Refine.hpp>
#include <HMP/Actions/Rotate.hpp>
#include <HMP/Actions/Paste.hpp>
#include <HMP/Actions/TransformAll.hpp>
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
	}

	void App::updateDagViewer()
	{
		m_dagViewerNeedsUpdate = true;
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

	void App::onPrintDebugInfo() const
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
				<< "name: " << m_dagNamer.nameOrUnknown(&element)
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

	bool App::onKeyPress(int _key, int _modifiers)
	{
		cinolib::KeyBinding key{ _key, _modifiers };
		// extrude
		if (key == c_kbExtrude)
		{
			onExtrude();
		}
		// copy
		else if (key == c_kbCopy)
		{
			onCopy();
		}
		// paste
		else if (key == c_kbPaste)
		{
			onPaste();
		}
		// load target mesh
		else if (key == c_kbLoadTarget)
		{
			onLoadTargetMesh();
		}
		// refine hexahedron
		else if (key == c_kbRefine)
		{
			onRefineElement(false);
		}
		// refine hexahedron twice
		else if (key == c_kbDoubleRefine)
		{
			onRefineElement(true);
		}
		// refine face
		else if (key == c_kbFaceRefine)
		{
			onRefineFace();
		}
		// delete hexahedron
		else if (key == c_kbDelete)
		{
			onDelete();
		}
		// rotate
		else if (key == c_kbRotate)
		{
			onRotate();
		}
		// make conformant
		else if (key == c_kbMakeConforming)
		{
			onMakeConformant();
		}
		// save tree
		else if (key == c_kbSave)
		{
			onSaveTree();
		}
		// save mesh
		else if (key == c_kbSaveMesh)
		{
			onSaveMesh();
		}
		// load tree
		else if (key == c_kbOpen)
		{
			onLoadTree();
		}
		// toggle target visibility
		else if (key == c_kbToggleTargetVisibility)
		{
			onToggleTargetVisibility();
		}
		// undo
		else if (key == c_kbUndo)
		{
			onUndo();
		}
		// redo
		else if (key == c_kbRedo)
		{
			onRedo();
		}
		// clear
		else if (key == c_kbClear)
		{
			onClear();
		}
		// print elements
		else if (key == c_kbPrintDebugInfo)
		{
			onPrintDebugInfo();
		}
		else
		{
			return false;
		}
		return true;
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

	void App::onLoadTargetMesh()
	{
		m_targetWidget.load();
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
		m_commander.apply(*new Actions::TransformAll{ _transform });
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

		m_targetWidget.onProjectRequest += [this](const Widgets::Target& _target) { onProjectToTarget(); };
		m_targetWidget.onMeshLoad += [this](const Widgets::Target& _target) { m_canvas.push(&_target.mesh(), false); };
		m_targetWidget.onMeshClear += [this](const Widgets::Target& _target) { m_canvas.pop(&_target.mesh()); };
		m_targetWidget.onApplyTransformToSource += [this](const Widgets::Target& _target, const Mat4& _transform) { onApplyTargetTransform(_transform); };

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

	void App::printKeyBindings()
	{
		std::cout << "------ App key bindings -------\n";
		cinolib::print_binding(c_kbExtrude.name().c_str(), "extrude");
		cinolib::print_binding(c_kbRefine.name().c_str(), "refine");
		cinolib::print_binding(c_kbDoubleRefine.name().c_str(), "refine twice");
		cinolib::print_binding(c_kbFaceRefine.name().c_str(), "refine face");
		cinolib::print_binding(c_kbDelete.name().c_str(), "delete");
		cinolib::print_binding(c_kbCopy.name().c_str(), "copy");
		cinolib::print_binding(c_kbPaste.name().c_str(), "paste");
		cinolib::print_binding(c_kbRotate.name().c_str(), "rotate");
		cinolib::print_binding(c_kbClear.name().c_str(), "clear");
		cinolib::print_binding(c_kbMakeConforming.name().c_str(), "make conforming");
		cinolib::print_binding(c_kbSave.name().c_str(), "save");
		cinolib::print_binding(c_kbOpen.name().c_str(), "open");
		cinolib::print_binding(c_kbSaveMesh.name().c_str(), "save mesh");
		cinolib::print_binding(c_kbLoadTarget.name().c_str(), "load target mesh");
		cinolib::print_binding(c_kbToggleTargetVisibility.name().c_str(), "toggle target visibility");
		cinolib::print_binding(c_kbUndo.name().c_str(), "undo");
		cinolib::print_binding(c_kbRedo.name().c_str(), "redo");
		cinolib::print_binding(c_kbPrintDebugInfo.name().c_str(), "print debug info");
		std::cout << "-------------------------------\n";
	}

	int App::launch()
	{
		printKeyBindings();
		return m_canvas.launch();
	}

}