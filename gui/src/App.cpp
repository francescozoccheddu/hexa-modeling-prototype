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
#include <utility>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Actions/Root.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Project.hpp>
#include <HMP/Actions/Refine.hpp>
#include <HMP/Actions/Paste.hpp>
#include <HMP/Actions/Transform.hpp>
#include <HMP/Utils/Serialization.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/Utils/HrDescriptions.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <cpputils/range/of.hpp>
#include <cpputils/range/index.hpp>
#include <sstream>
#include <array>
#include <iomanip>
#include <filesystem>
#include <ctime>
#include <cstring>
#include <cinolib/geometry/plane.h>

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
#include <HMP/Gui/DagViewer/createLayout.hpp>
#endif

namespace HMP::Gui
{

	void App::printKeyBindings()
	{
		std::cout << "------ App key bindings -------\n";
		cinolib::print_binding(c_kbExtrudeFace.name(), "extrude face");
		cinolib::print_binding(c_kbExtrudeEdge.name(), "extrude edge");
		cinolib::print_binding(c_kbExtrudeVertex.name(), "extrude vertex");
		cinolib::print_binding(c_kbExtrudeSelected.name(), "extrude selected");
		cinolib::print_binding(c_kbRefine.name(), "refine");
		cinolib::print_binding(c_kbDoubleRefine.name(), "refine twice");
		cinolib::print_binding(c_kbFaceRefine.name(), "refine face");
		cinolib::print_binding(c_kbDelete.name(), "delete");
		cinolib::print_binding(c_kbCopy.name(), "copy");
		cinolib::print_binding(c_kbPasteFace.name(), "paste face");
		cinolib::print_binding(c_kbPasteEdge.name(), "paste edge");
		cinolib::print_binding(c_kbPasteVertex.name(), "paste vertex");
		cinolib::print_binding(c_kbClear.name(), "clear");
		cinolib::print_binding(c_kbMakeConforming.name(), "make conforming");
		cinolib::print_binding(c_kbSelectVertex.name(), "select vertex");
		cinolib::print_binding(c_kbSelectEdge.name(), "select edge vertices");
		cinolib::print_binding(c_kbSelectUpEdge.name(), "select adjacent edge vertices");
		cinolib::print_binding(c_kbSelectFace.name(), "select face vertices");
		cinolib::print_binding(c_kbSelectUpFace.name(), "select adjacent face vertices");
		cinolib::print_binding(c_kbSelectPoly.name(), "select poly vertices");
		cinolib::print_binding(cinolib::KeyBinding::mod_names(c_kmodSelectAdd), "remove from selection (hold down)");
		cinolib::print_binding(cinolib::KeyBinding::mod_names(c_kmodSelectRemove), "add to selection (hold down)");
		cinolib::print_binding(c_kbSelectAll.name(), "select all vertices");
		cinolib::print_binding(c_kbDeselectAll.name(), "deselect all vertices");
		cinolib::print_binding(c_kbDirectTranslation.name(), "translate selected vertices");
		cinolib::print_binding(c_kbDirectScale.name(), "scale selected vertices");
		cinolib::print_binding(cinolib::KeyBinding::key_name(c_kbDirectEditX), "lock direct edit along X (hold down)");
		cinolib::print_binding(cinolib::KeyBinding::key_name(c_kbDirectEditY), "lock direct edit along Y (hold down)");
		cinolib::print_binding(cinolib::KeyBinding::key_name(c_kbDirectEditZ), "lock direct edit along Z (hold down)");
		cinolib::print_binding(c_kbDirectRotation.name(), "rotate selected vertices");
		cinolib::print_binding(c_kbCancelDirectEdit.name(), "cancel direct edit");
		cinolib::print_binding(c_kbSave.name(), "save");
		cinolib::print_binding(c_kbSaveNew.name(), "save new");
		cinolib::print_binding(c_kbOpen.name(), "open");
		cinolib::print_binding(c_kbLoadTarget.name(), "load target mesh");
		cinolib::print_binding(c_kbToggleTargetVisibility.name(), "toggle target visibility");
		cinolib::print_binding(c_kbUndo.name(), "undo");
		cinolib::print_binding(c_kbRedo.name(), "redo");
		cinolib::print_binding(c_kbAddPathEdge.name(), "add path edge");
		cinolib::print_binding(c_kbRemovePathEdge.name(), "remove path edge");
		cinolib::print_binding(c_kbRedo.name(), "redo");
		cinolib::print_binding(c_kbPrintDebugInfo.name(), "print debug info");
		std::cout << "-------------------------------\n";
	}

	// actions

	void App::onActionApplied()
	{
		m_mesher.updateMesh();
		m_vertEditWidget.updateCentroid();
		m_mouse.element = nullptr;
		updateMouse();
		requestDagViewerUpdate();
		m_canvas.refit_scene();
	}

	void App::applyAction(Commander::Action& _action)
	{
		m_vertEditWidget.applyAction();
		m_commander.apply(_action);
		onActionApplied();
	}

	void App::requestDagViewerUpdate()
	{
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		m_dagViewerNeedsUpdate = true;
#endif
	}

	// mesher events

	void App::onElementRemove(const HMP::Dag::Element& _element, const std::vector<Id>& _removedVids)
	{
		if (m_mouse.element == &_element)
		{
			m_mouse.element = nullptr;
		}
		if (m_copy.element == &_element)
		{
			m_copy.element = nullptr;
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
			m_dagViewerWidget.copied = nullptr;
#endif
		}
		m_vertEditWidget.remove(_removedVids);
	}

	void App::onElementRemoved(const HMP::Dag::Element& _element, const std::vector<Id>& _removedVids, bool _actuallyRemovedVids)
	{
		if (_actuallyRemovedVids)
		{
			Id meshVertCount{ m_mesh.num_verts() + toId(_removedVids.size()) };
			for (const Id vid : _removedVids)
			{
				m_vertEditWidget.replace(--meshVertCount, vid);
			}
		}
	}

	void App::onClearElements()
	{
		m_mouse.element = nullptr;
		m_copy.element = nullptr;
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		m_dagViewerWidget.copied = nullptr;
#endif
		m_vertEditWidget.clear();
	}

	// vert edit events

	void App::onApplyVertEdit(const std::vector<Id>& _vids, const Mat4& _transform)
	{
		applyAction(*new Actions::Transform{ _transform, _vids });
	}

	void App::onVertEditPendingActionChanged()
	{
		if (m_vertEditWidget.pendingAction())
		{
			m_commander.unapplied().clear();
		}
	}

	// save events

	void App::onExportMesh(const std::string& _filename)
	{
		m_mesh.save(_filename.c_str());
	}

	void App::onSaveState(const std::string& _filename)
	{
		std::ofstream file;
		file.open(_filename);
		HMP::Utils::Serialization::Serializer serializer{ file };
		HMP::Dag::Utils::serialize(serializer, *m_project.root());
		serializer << toI(m_mesh.num_verts());
		for (const Vec& vert : m_mesh.vector_verts())
		{
			serializer << vert;
		}
		m_targetWidget.serialize(serializer);
		file.close();
	}

	void App::onLoadState(const std::string& _filename)
	{
		std::ifstream file;
		file.open(_filename);
		HMP::Utils::Serialization::Deserializer deserializer{ file };
		HMP::Dag::Element& root = HMP::Dag::Utils::deserialize(deserializer).element();
		std::vector<Vec> verts(deserializer.get<I>());
		for (Vec& vert : verts)
		{
			deserializer >> vert;
		}
		m_targetWidget.deserialize(deserializer);
		file.close();
		applyAction(*new Actions::Root{ root, verts });
		m_canvas.reset_camera();
	}

	// canvas events

	void App::onCameraChanged()
	{
		m_directVertEditWidget.cancel();
		updateMouse();
	}

	bool App::onMouseLeftClicked(int _modifiers)
	{
		if (m_directVertEditWidget.pending())
		{
			m_directVertEditWidget.apply();
			return true;
		}
		return false;
	}

	bool App::onMouseRightClicked(int _modifiers)
	{
		if (m_directVertEditWidget.pending())
		{
			m_directVertEditWidget.cancel();
			return true;
		}
		return false;
	}

	bool App::onKeyPressed(int _key, int _modifiers)
	{
		cinolib::KeyBinding key{ _key, _modifiers };
		// extrude
		if (key == c_kbExtrudeFace)
		{
			onExtrude(Dag::Extrude::ESource::Face);
		}
		else if (key == c_kbExtrudeEdge)
		{
			onExtrude(Dag::Extrude::ESource::Edge);
		}
		else if (key == c_kbExtrudeVertex)
		{
			onExtrude(Dag::Extrude::ESource::Vertex);
		}
		// extrude selected
		else if (key == c_kbExtrudeSelected)
		{
			onExtrudeSelected();
		}
		// copy
		else if (key == c_kbCopy)
		{
			onCopy();
		}
		// paste
		else if (key == c_kbPasteFace)
		{
			onPaste(Dag::Extrude::ESource::Face);
		}
		else if (key == c_kbPasteEdge)
		{
			onPaste(Dag::Extrude::ESource::Edge);
		}
		else if (key == c_kbPasteVertex)
		{
			onPaste(Dag::Extrude::ESource::Vertex);
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
		// make conformant
		else if (key == c_kbMakeConforming)
		{
			onMakeConformant();
		}
		// save tree
		else if (key == c_kbSave)
		{
			onSaveState();
		}
		// save mesh
		else if (key == c_kbSaveNew)
		{
			onSaveNewState();
		}
		// load tree
		else if (key == c_kbOpen)
		{
			onLoadState();
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
		// direct manipulation
		else if (key == c_kbDirectTranslation)
		{
			m_directVertEditWidget.request(Widgets::DirectVertEdit::EKind::Translation, m_mouse.position);
		}
		else if (key == c_kbDirectScale)
		{
			m_directVertEditWidget.request(Widgets::DirectVertEdit::EKind::Scale, m_mouse.position);
		}
		else if (key == c_kbDirectRotation)
		{
			m_directVertEditWidget.request(Widgets::DirectVertEdit::EKind::Rotation, m_mouse.position);
		}
		else if (key == c_kbCancelDirectEdit)
		{
			m_directVertEditWidget.cancel();
		}
		// selection
		else if (key == c_kbSelectVertex)
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Set);
		}
		else if (key == c_kbSelectEdge)
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Set);
		}
		else if (key == c_kbSelectUpEdge)
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Set);
		}
		else if (key == c_kbSelectFace)
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Set);
		}
		else if (key == c_kbSelectUpFace)
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Set);
		}
		else if (key == c_kbSelectPoly)
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Set);
		}
		else if (key == (c_kbSelectVertex | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Add);
		}
		else if (key == (c_kbSelectEdge | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Add);
		}
		else if (key == (c_kbSelectUpEdge | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Add);
		}
		else if (key == (c_kbSelectFace | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Add);
		}
		else if (key == (c_kbSelectUpFace | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Add);
		}
		else if (key == (c_kbSelectPoly | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Add);
		}
		else if (key == (c_kbSelectVertex | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Vertex, ESelectionMode::Remove);
		}
		else if (key == (c_kbSelectEdge | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Edge, ESelectionMode::Remove);
		}
		else if (key == (c_kbSelectUpEdge | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::UpEdge, ESelectionMode::Remove);
		}
		else if (key == (c_kbSelectFace | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Remove);
		}
		else if (key == (c_kbSelectUpFace | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::UpFace, ESelectionMode::Remove);
		}
		else if (key == (c_kbSelectPoly | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Remove);
		}
		else if (key == c_kbDeselectAll)
		{
			onSelectAll(false);
		}
		else if (key == c_kbSelectAll)
		{
			onSelectAll(true);
		}
		else if (key == c_kbAddPathEdge)
		{
			onSetPathEdge(true);
		}
		else if (key == c_kbRemovePathEdge)
		{
			onSetPathEdge(false);
		}
		else
		{
			return false;
		}
		return true;
	}

	void App::onSetPathEdge(bool _add)
	{
		Vec point;
		if (m_canvas.unproject(m_mouse.position, point))
		{
			if (m_targetWidget.hasMesh() && m_targetWidget.visible())
			{
				m_projectionWidget.setTargetPathEdgeAtPoint(point, _add);
			}
			else
			{
				m_projectionWidget.setSourcePathEdgeAtPoint(point, _add);
			}
		}
	}

	bool App::onMouseMoved(double _x, double _y)
	{
		m_mouse.position = cinolib::vec2d{ _x, _y };
		updateMouse();
		return m_directVertEditWidget.pending();
	}

	void App::onDrawControls()
	{
		// names
		{
			ImGui::Checkbox("Show element names", &m_options.showNames);
			ImGui::SameLine();
			if (ImGui::Button("Reset names"))
			{
				m_dagNamer.reset();
			}
		}
		if (ImGui::Button("Crash me!"))
		{
			throw std::runtime_error{ "user requested crash" };
		}
	}

	void App::onDrawCustomGui()
	{
		const ImVec4 warningColor{ Utils::Controls::toImGui(c_warningTextColor) };
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		using namespace Utils::Drawing;
		const ImU32 colorU32{ toU32(c_overlayColor) };
		const ImU32 mutedColorU32{ toU32(c_mutedOverlayColor) };
		if (m_copy.element && !m_mouse.element)
		{
			const Id pid{ m_mesher.elementToPid(*m_copy.element) };
			const Dag::Extrude& extrude{ m_copy.element->parents.cast<const Dag::Extrude&>().single() };
			const ImVec2 center{ project(m_canvas, m_mesh.poly_centroid(pid)) };
			for (const auto& [parent, fi] : extrude.parents.zip(extrude.fis))
			{
				const QuadVertIds parentFidVids{ Meshing::Utils::faceVids(parent, fi) };
				const QuadVerts parentFidVerts{ Meshing::Utils::verts(m_mesh, parentFidVids) };
				const Vec parentFidCentroid{ cpputils::range::of(parentFidVerts).sum() / 4.0 };
				const ImVec2 parentFidCenter{ project(m_canvas, parentFidCentroid) };
				dashedLine(drawList, parentFidCenter, center, mutedColorU32, 1.5f);
			}
			if (!m_options.showNames || !m_mesh.poly_is_on_surf(pid))
			{
				circle(drawList, center, 4.0f, m_mouse.element == m_copy.element ? colorU32 : mutedColorU32, 1.5f);
			}
			const Dag::Element& firstParent{ extrude.parents.first() };
			const Id firstVid{ firstParent.vids[extrude.firstVi] };
			const QuadVertIds firstParentVids{ Meshing::Utils::align(Meshing::Utils::faceVids(firstParent, extrude.fis[0]), firstVid, extrude.clockwise) };
			const ImVec2 eVert1{ project(m_canvas, m_mesh.vert(firstParentVids[0])) };
			const ImVec2 eVert2{ project(m_canvas, m_mesh.vert(firstParentVids[1])) };
			dashedLine(drawList, eVert1, eVert2, mutedColorU32, 4.0f);
			circleFilled(drawList, eVert1, 4.0f, mutedColorU32);
		}
		if (m_mouse.element)
		{
			const Id pid{ m_mesher.elementToPid(*m_mouse.element) };
			const Id vid{ m_mesh.poly_vert_id(pid, m_mouse.vertOffset) };
			const Id fid{ m_mesh.poly_face_id(pid, m_mouse.faceOffset) };
			const Id upFid{ m_mesh.poly_face_id(pid, m_mouse.upFaceOffset) };
			const Id upEid{ m_mesh.face_shared_edge(fid, upFid) };
			const ImVec2 pidCenter{ project(m_canvas, m_mesh.poly_centroid(pid)) };
			for (const Id adjPid : m_mesh.adj_p2p(pid))
			{
				const Id adjFid{ static_cast<Id>(m_mesh.poly_shared_face(pid, adjPid)) };
				const ImVec2 adjFidCenter{ project(m_canvas, m_mesh.face_centroid(adjFid)) };
				const ImVec2 adjPidCenter{ project(m_canvas, m_mesh.poly_centroid(adjPid)) };
				if (m_options.showNames)
				{
					const Dag::Element& element{ m_mesher.pidToElement(adjPid) };
					text(drawList, m_dagNamer(&element).c_str(), adjPidCenter, 20.0f, colorU32);
				}
				else
				{
					circle(drawList, adjPidCenter, 4.0f, colorU32, 1.5f);
				}
				dashedLine(drawList, adjFidCenter, pidCenter, colorU32, 1.5f);
			}
			if (m_options.showNames)
			{
				text(drawList, m_dagNamer(m_mouse.element).c_str(), pidCenter, 20.0f, colorU32);
			}
			else
			{
				circle(drawList, pidCenter, 4.0f, colorU32, 1.5f);
			}
			for (const Id eid : m_mesh.adj_f2e(fid))
			{
				const ImVec2 eVert1{ project(m_canvas, m_mesh.vert(m_mesh.edge_vert_id(eid, 0))) };
				const ImVec2 eVert2{ project(m_canvas, m_mesh.vert(m_mesh.edge_vert_id(eid, 1))) };
				dashedLine(drawList, eVert1, eVert2, eid == upEid ? colorU32 : mutedColorU32, 4.0f);
			}
			{
				const ImVec2 vert{ project(m_canvas, m_mesh.vert(vid)) };
				circleFilled(drawList, vert, 4.0f, colorU32);
			}
		}
		else if (m_options.showNames)
		{
			for (Id pid{}; pid < m_mesh.num_polys(); pid++)
			{
				if (m_mesh.poly_is_on_surf(pid))
				{
					const ImVec2 center{ project(m_canvas, m_mesh.poly_centroid(pid)) };
					const Dag::Element& element{ m_mesher.pidToElement(pid) };
					text(drawList, m_dagNamer(&element).c_str(), center, 20.0f, mutedColorU32);
				}
			}
		}
		if (m_mouse.element)
		{
			std::ostringstream stream{};
			stream
				<< "Hovering "
				<< Utils::HrDescriptions::name(*m_mouse.element, m_dagNamer)
				<< " ("
				<< "faces " << Utils::HrDescriptions::describe(std::vector<Id>{ m_mouse.faceOffset, m_mouse.upFaceOffset })
				<< ", vert " << m_mouse.vertOffset
				<< ")";
			ImGui::TextDisabled("%s", stream.str().c_str());
		}
		if (m_copy.element)
		{
			std::ostringstream stream{};
			stream
				<< "Copied"
				<< " " << Utils::HrDescriptions::name(*m_copy.element, m_dagNamer);
			ImGui::TextDisabled("%s", stream.str().c_str());
		}
		if (!m_vertEditWidget.empty())
		{
			const char* verticesLit{ m_vertEditWidget.vids().size() == 1 ? "vertex" : "vertices" };
			const int vertexCount{ static_cast<int>(m_vertEditWidget.vids().size()) };
			ImGui::TextDisabled("%d %s selected", vertexCount, verticesLit);
			if (m_directVertEditWidget.pending())
			{
				switch (m_directVertEditWidget.kind())
				{
					case Widgets::DirectVertEdit::EKind::Rotation:
					{
						const Vec rot{ m_vertEditWidget.transform().rotation };
						ImGui::TextColored(warningColor, "Rotating %d %s by %1.f,%1.f,%1.f degrees via direct manipulation", vertexCount, verticesLit, rot.x(), rot.y(), rot.z());
					}
					break;
					case Widgets::DirectVertEdit::EKind::Scale:
					{
						const Vec scl{ m_vertEditWidget.transform().scale * 100.0 };
						ImGui::TextColored(warningColor, "Scaling %d %s by %2.f,%2.f,%2.f%% via direct manipulation", vertexCount, verticesLit, scl.x(), scl.y(), scl.z());
					}
					break;
					case Widgets::DirectVertEdit::EKind::Translation:
					{
						const Vec trs{ m_vertEditWidget.transform().translation };
						ImGui::TextColored(warningColor, "Translating %d %s by %3.f,%3.f,%.3f via direct manipulation", vertexCount, verticesLit, trs.x(), trs.y(), trs.z());
					}
					break;
				}
			}
		}
	}

	void App::onDagViewerDraw()
	{
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		if (m_dagViewerNeedsUpdate)
		{
			m_dagViewerNeedsUpdate = false;
			if (m_project.root())
			{
				m_dagViewerWidget.layout() = DagViewer::createLayout(*m_project.root());
			}
			m_dagViewerWidget.resetView();
		}
#endif
	}

	void App::updateMouse()
	{
		HMP::Dag::Element* const lastElement{ m_mouse.element };
		const Id lastFaceOffset{ m_mouse.faceOffset }, lastUpFaceOffset{ m_mouse.upFaceOffset }, lastVertOffset{ m_mouse.vertOffset };
		m_mouse.element = nullptr;
		m_mouse.faceOffset = m_mouse.vertOffset = noId;
		if (!m_directVertEditWidget.pending())
		{
			Id pid, fid, eid, vid;
			const cinolib::Ray ray{ m_canvas.eye_to_mouse_ray() };
			if (m_mesher.pick(ray.begin(), ray.dir(), pid, fid, eid, vid, !m_canvas.camera.projection.perspective))
			{
				m_mouse.element = &m_mesher.pidToElement(pid);
				m_mouse.faceOffset = m_mesh.poly_face_offset(pid, fid);
				const Id upFid{ Meshing::Utils::adjFidInPidByEidAndFid(m_mesh, pid, fid, eid) };
				m_mouse.upFaceOffset = m_mesh.poly_face_offset(pid, upFid);
				m_mouse.vertOffset = m_mesh.poly_vert_offset(pid, vid);
				m_mouse.fi = Meshing::Utils::fi(*m_mouse.element, cpputils::range::of(m_mesh.face_verts_id(fid)).toArray<4>());
				m_mouse.ei = Meshing::Utils::ei(*m_mouse.element, cpputils::range::of(m_mesh.edge_vert_ids(eid)).toArray<2>());
				m_mouse.vi = Meshing::Utils::vi(*m_mouse.element, vid);
			}
		}
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		m_dagViewerWidget.highlight = m_mouse.element;
#endif
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
		const bool lockX{ glfwGetKey(m_canvas.window, c_kbDirectEditX) == GLFW_PRESS };
		const bool lockY{ glfwGetKey(m_canvas.window, c_kbDirectEditY) == GLFW_PRESS };
		const bool lockZ{ glfwGetKey(m_canvas.window, c_kbDirectEditZ) == GLFW_PRESS };
		m_directVertEditWidget.update(m_mouse.position, lockX, lockY, lockZ);
	}

	// Commands

	std::string App::getDebugInfo() const
	{
		std::ostringstream ss{};
		ss << "---- Elements\n";
		for (const auto& [element, pid] : m_mesher)
		{
			ss
				<< "name: " << m_dagNamer.nameOrUnknown(&element)
				<< " pid: " << pid
				<< " centroid: " << Utils::HrDescriptions::describe(m_mesh.poly_centroid(pid))
				<< " vids: " << Utils::HrDescriptions::describe(m_mesh.adj_p2v(pid))
				<< " eids: " << Utils::HrDescriptions::describe(m_mesh.adj_p2e(pid))
				<< " fids: " << Utils::HrDescriptions::describe(m_mesh.adj_p2f(pid))
				<< " pids: " << Utils::HrDescriptions::describe(m_mesh.adj_p2p(pid))
				<< " winding: " << Utils::HrDescriptions::describe(m_mesh.poly_faces_winding(pid))
				<< "\n";
		}
		ss << "---- Faces\n";
		for (Id fid{}; fid < m_mesh.num_faces(); fid++)
		{
			ss
				<< "fid: " << fid
				<< " centroid: " << Utils::HrDescriptions::describe(m_mesh.face_centroid(fid))
				<< " vids: " << Utils::HrDescriptions::describe(m_mesh.adj_f2v(fid))
				<< " eids: " << Utils::HrDescriptions::describe(m_mesh.adj_f2e(fid))
				<< " fids: " << Utils::HrDescriptions::describe(m_mesh.adj_f2f(fid))
				<< " pids: " << Utils::HrDescriptions::describe(m_mesh.adj_f2p(fid))
				<< " normal: " << Utils::HrDescriptions::describe(m_mesh.face_data(fid).normal)
				<< "\n";
		}
		ss << "---- Edges\n";
		for (Id eid{}; eid < m_mesh.num_edges(); eid++)
		{
			ss
				<< "eid: " << eid
				<< " midpoint: " << Utils::HrDescriptions::describe(Meshing::Utils::midpoint(m_mesh, eid))
				<< " vids: " << Utils::HrDescriptions::describe(m_mesh.adj_e2v(eid))
				<< " eids: " << Utils::HrDescriptions::describe(m_mesh.adj_e2e(eid))
				<< " fids: " << Utils::HrDescriptions::describe(m_mesh.adj_e2f(eid))
				<< " pids: " << Utils::HrDescriptions::describe(m_mesh.adj_e2p(eid))
				<< "\n";
		}
		ss << "---- Vertices\n";
		for (Id vid{}; vid < m_mesh.num_verts(); vid++)
		{
			ss
				<< "vid: " << vid
				<< " position: " << Utils::HrDescriptions::describe(m_mesh.vert(vid))
				<< " vids: " << Utils::HrDescriptions::describe(m_mesh.adj_v2v(vid))
				<< " eids: " << Utils::HrDescriptions::describe(m_mesh.adj_v2e(vid))
				<< " fids: " << Utils::HrDescriptions::describe(m_mesh.adj_v2f(vid))
				<< " pids: " << Utils::HrDescriptions::describe(m_mesh.adj_v2p(vid))
				<< "\n";
		}
		return ss.str();
	}

	void App::onPrintDebugInfo() const
	{
		std::cout
			<< "-------- PRINT DEBUG INFO --------\n"
			<< getDebugInfo()
			<< "----------------------------------"
			<< std::endl;
	}

	bool App::hoveredExtrudeElements(Dag::Extrude::ESource _source, cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, cpputils::collections::FixedVector<I, 3>& _fis, I& _firstVi, bool& _clockwise)
	{
		cpputils::collections::FixedVector<Id, 3> pids, fids;
		if (m_mouse.element)
		{
			pids.addLast(m_mesher.elementToPid(*m_mouse.element));
			fids.addLast(m_mesh.poly_face_id(pids[0], m_mouse.faceOffset));
			const Id commVid{ m_mesh.poly_vert_id(pids[0], m_mouse.vertOffset) };
			const Id firstEid{ m_mesh.face_shared_edge(fids[0], m_mesh.poly_face_id(pids[0], m_mouse.upFaceOffset)) };
			if (_source != Dag::Extrude::ESource::Face)
			{
				const cinolib::Plane firstPlane{
					m_mesh.face_centroid(fids[0]),
					m_mesh.poly_face_normal(pids[0], fids[0])
				};
				for (const Id adjFid : m_mesh.adj_e2f(firstEid))
				{
					Id adjPid;
					if (m_mesh.face_is_visible(adjFid, adjPid)
						&& adjPid != pids[0]
						&& firstPlane.point_plane_dist_signed(m_mesh.face_centroid(adjFid)) > 0)
					{
						if (fids.size() == 2)
						{
							const auto edgeVec{ [&](const Id _pid, const Id _fid) {
								return m_mesh.edge_vec(m_mesh.edge_id(commVid, m_mesh.poly_vert_opposite_to(_pid, _fid, commVid)), true);
							} };
							const Vec firstEdge{ edgeVec(pids[0], fids[0]) };
							const Vec currSecondEdge{ edgeVec(pids[1], fids[1]) };
							const Vec candSecondEdge{ edgeVec(adjPid, adjFid) };
							if (firstEdge.dot(candSecondEdge) > firstEdge.dot(currSecondEdge))
							{
								pids[1] = adjPid;
								fids[1] = adjFid;
							}
						}
						else
						{
							pids.addLast(adjPid);
							fids.addLast(adjFid);
						}
					}
				}
				if (fids.size() != 2)
				{
					return false;
				}
				if (_source == Dag::Extrude::ESource::Vertex)
				{
					for (const Id adjFid : m_mesh.adj_f2f(fids[0]))
					{
						Id adjPid;
						if (m_mesh.face_is_visible(adjFid, adjPid)
							&& adjPid != pids[0] && adjPid != pids[1]
							&& adjFid != fids[0] && adjFid != fids[1]
							&& m_mesh.face_contains_vert(adjFid, commVid)
							&& m_mesh.faces_are_adjacent(adjFid, fids[1]))
						{
							if (fids.size() == 3)
							{
								return false;
							}
							pids.addLast(adjPid);
							fids.addLast(adjFid);
						}
					}
					if (fids.size() != 3)
					{
						return false;
					}
				}
			}
			_clockwise = Meshing::Utils::isEdgeCW(m_mesh, pids[0], fids[0], commVid, firstEid);
			_elements = cpputils::range::of(pids).map([&](Id _pid) {
				return &m_mesher.pidToElement(_pid);
			}).toFixedVector<3>();
			_fis = cpputils::range::zip(fids, _elements).map([&](const auto& _fidAndElement) {
				const auto& [fid, element] {_fidAndElement};
			const QuadVertIds vids{ cpputils::range::of(m_mesh.face_verts_id(fid)).template toArray<4>() };
			return Meshing::Utils::fi(*element, vids);
			}).toFixedVector<3>();
			_firstVi = m_mouse.vi;
			return true;
		}
		return false;
	}

	void App::onExtrude(Dag::Extrude::ESource _source)
	{
		cpputils::collections::FixedVector<Dag::Element*, 3> elements;
		cpputils::collections::FixedVector<I, 3> fis;
		I firstVi;
		bool clockwise;
		if (hoveredExtrudeElements(_source, elements, fis, firstVi, clockwise))
		{
			applyAction(*new Actions::Extrude{ elements, fis, firstVi, clockwise });
		}
	}

	void App::onExtrudeSelected()
	{
		const std::vector<Id> vids{ m_vertEditWidget.vids().toVector() };
		if (vids.size() != 4)
		{
			return;
		}
		const Id fid{ static_cast<Id>(m_mesh.face_id(vids)) };
		if (fid == noId)
		{
			return;
		}
		Id pid;
		if (!m_mesh.face_is_visible(fid, pid))
		{
			return;
		}
		Dag::Element& element{ m_mesher.pidToElement(pid) };
		const I fi{ Meshing::Utils::fi(element, cpputils::range::of(m_mesh.face_verts_id(fid)).toArray<4>()) };
		const I vi{ Meshing::Utils::vi(element, vids[0]) };
		Actions::Extrude& action{ *new Actions::Extrude{ {&element}, {fi}, vi, false } };
		applyAction(action);
		const Id newPid{ m_mesher.elementToPid(action.operation().children.single()) };
		const Id newFid{ m_mesh.poly_face_opposite_to(newPid, fid) };
		m_vertEditWidget.clear();
		m_vertEditWidget.add(m_mesh.face_verts_id(newFid));
	}

	void App::onCopy()
	{
		if (m_mouse.element && m_mouse.element->parents.isSingle() && m_mouse.element->parents.first().primitive == Dag::Operation::EPrimitive::Extrude)
		{
			m_copy.element = m_mouse.element;
		}
		else
		{
			m_copy.element = nullptr;
		}
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		m_dagViewerWidget.copied = m_copy.element;
#endif
	}

	void App::onPaste(Dag::Extrude::ESource _source)
	{
		if (m_copy.element)
		{
			cpputils::collections::FixedVector<Dag::Element*, 3> elements;
			cpputils::collections::FixedVector<I, 3> fis;
			I firstVi;
			bool clockwise;
			if (hoveredExtrudeElements(_source, elements, fis, firstVi, clockwise))
			{
				applyAction(*new Actions::Paste{ elements, fis, firstVi, clockwise, static_cast<const Dag::Extrude&>(m_copy.element->parents.single()) });
			}
		}
	}

	void App::onRefineElement(bool _twice)
	{
		if (m_mouse.element)
		{
			applyAction(*new Actions::Refine{ *m_mouse.element, m_mouse.fi, m_mouse.vi, Refinement::EScheme::Subdivide3x3, _twice ? 2u : 1u });
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
			applyAction(*new Actions::Delete{ *m_mouse.element });
		}
	}

	void App::onRefineFace()
	{
		if (m_mouse.element)
		{
			applyAction(*new Actions::Refine{ *m_mouse.element, m_mouse.fi, m_mouse.vi, Refinement::EScheme::Inset });
		}
	}

	void App::onMakeConformant()
	{
		applyAction(*new Actions::MakeConforming());
	}

	void App::onSaveState()
	{
		m_saveWidget.requestSave();
	}

	void App::onSaveNewState()
	{
		m_saveWidget.requestSaveNew();
	}

	void App::onLoadState()
	{
		m_saveWidget.requestLoad();
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

	void App::onProjectToTarget(const cinolib::Polygonmesh<>& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options)
	{
		applyAction(*new Actions::Project{ _target, _pointFeats, _pathFeats, _options });
	}

	void App::onApplyTargetTransform(const Mat4& _transform)
	{
		applyAction(*new Actions::Transform{ _transform });
		m_canvas.reset_camera();
	}

	void App::onUndo()
	{
		if (m_commander.canUndo())
		{
			m_vertEditWidget.applyAction();
			m_commander.undo();
			onActionApplied();
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
			m_vertEditWidget.applyAction();
			m_commander.redo();
			onActionApplied();
		}
		else
		{
			std::cout << "cannot redo" << std::endl;
		}
	}

	void App::onClear()
	{
		Dag::Element& root{ *new Dag::Element{} };
		root.vids = { 0,1,2,3,4,5,6,7 };
		applyAction(*new Actions::Root{ root, {
			Vec{-1,-1,-1}, Vec{+1,-1,-1}, Vec{+1,+1,-1}, Vec{-1,+1,-1},
			Vec{-1,-1,+1}, Vec{+1,-1,+1}, Vec{+1,+1,+1}, Vec{-1,+1,+1},
		} });
	}

	void App::onSelect(ESelectionSource _source, ESelectionMode _mode)
	{
		if (m_mouse.element)
		{
			const Id pid{ m_mesher.elementToPid(*m_mouse.element) };
			const Id vid{ m_mesh.poly_vert_id(pid, m_mouse.vertOffset) };
			const Id fid{ m_mesh.poly_face_id(pid, m_mouse.faceOffset) };
			const Id upFid{ m_mesh.poly_face_id(pid, m_mouse.upFaceOffset) };
			const Id eid{ m_mesh.face_shared_edge(fid, upFid) };
			std::vector<Id> vids{};
			switch (_source)
			{
				case ESelectionSource::Vertex:
					vids = { vid };
					break;
				case ESelectionSource::Edge:
					vids = m_mesh.edge_vert_ids(eid);
					break;
				case ESelectionSource::Face:
					vids = m_mesh.face_verts_id(fid);
					break;
				case ESelectionSource::UpFace:
					vids = m_mesh.face_verts_id(upFid);
					break;
				case ESelectionSource::UpEdge:
					vids = { vid, m_mesh.poly_vert_opposite_to(pid, fid, vid) };
					break;
				case ESelectionSource::Poly:
					vids = m_mesh.poly_verts_id(pid);
					break;
			}
			if (_mode == ESelectionMode::Set)
			{
				m_vertEditWidget.clear();
			}
			if (_mode == ESelectionMode::Remove)
			{
				m_vertEditWidget.remove(vids);
			}
			else
			{
				m_vertEditWidget.add(vids);
			}
		}
	}

	void App::onSelectAll(bool _selected)
	{
		if (_selected)
		{
			std::vector<Id> vids(toI(m_mesh.num_verts()));
			for (I i{}; i < vids.size(); i++)
			{
				vids[i] = toId(i);
			}
			m_vertEditWidget.add(vids);
		}
		else
		{
			m_vertEditWidget.clear();
		}
	}

	void App::onFilesDropped(const std::vector<std::string>& _files)
	{
		if (_files.size() == 1)
		{
			loadTargetMeshOrProjectFile(_files[0]);
		}
	}

	void App::loadTargetMeshOrProjectFile(const std::string& _file)
	{
		static const std::unordered_set<std::string> targetMeshExts{ ".off", ".obj", ".stl" }, projectExts{ ".hmp" };
		std::string ext{ std::filesystem::path{_file}.extension().string() };
		for (char& c : ext) c = std::tolower(c);
		if (projectExts.contains(ext))
		{
			m_saveWidget.requestLoad(_file);
		}
		else if (targetMeshExts.contains(ext))
		{
			m_targetWidget.load(_file);
		}
		else
		{
			std::cerr << "unknown extension '" << ext << "'" << std::endl;
			std::cout << "only *.off, *.obj and *.stl target mesh files and *.hmp project files are supported" << std::endl;
		}
	}

	// launch

	App::App():
		m_project{}, m_canvas{ 700, 600, 13, 1.0f }, m_mesher{ m_project.mesher() }, m_mesh{ m_mesher.mesh() }, m_commander{ m_project.commander() },
		m_dagNamer{}, m_commanderWidget{ m_commander, m_dagNamer, m_vertEditWidget }, m_axesWidget{}, m_targetWidget{ m_mesh }, m_vertEditWidget{ m_mesher },
		m_directVertEditWidget{ m_vertEditWidget, m_canvas }, m_saveWidget{}, m_projectionWidget{ m_targetWidget, m_commander, m_mesher, m_vertEditWidget }
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		, m_dagViewerWidget{ m_mesher, m_dagNamer }, m_dagViewerNeedsUpdate{ true }
#endif
#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
		, m_ae3d2ShapeExporter{ m_mesh, m_canvas.camera }
#endif
	{

		{
#ifdef HMP_GUI_LIGHT_THEME
			ImGui::StyleColorsLight();
			static constexpr float styleHue{ 213.0f / 360.0f };
#else
			ImGui::StyleColorsDark();
			static constexpr float styleHue{ 213.0f / 360.0f };
#endif
			ImGuiStyle& style{ ImGui::GetStyle() };
			for (ImVec4& color : style.Colors)
			{
				float h, s, v;
				ImGui::ColorConvertRGBtoHSV(color.x, color.y, color.z, h, s, v);
				h += c_hue - styleHue;
				h = static_cast<float>(Utils::Transform::wrapAngle(static_cast<Real>(h * 360.0f))) / 360.0f;
				ImGui::ColorConvertHSVtoRGB(h, s, v, color.x, color.y, color.z);
			}
		}


		glfwSetWindowTitle(m_canvas.window, "hexa-modeling-prototype");

		m_canvas.background = c_backgroundColor;
		m_canvas.key_bindings.reset_camera = GLFW_KEY_P;
		m_canvas.key_bindings.store_camera = cinolib::KeyBindings::no_key_binding();
		m_canvas.key_bindings.restore_camera = cinolib::KeyBindings::no_key_binding();
		m_mesher.polyMarkerSet().color() = c_selectedPolyColor;
		m_mesher.faceMarkerSet().color() = c_selectedFaceColor;
		m_mesher.polyColor() = c_polyColor;
		m_mesher.edgeColor() = c_edgeColor;

		m_mesher.onElementRemove += [this](const HMP::Dag::Element& _element, const Meshing::Mesher::RemovedIds& _removedIds) { onElementRemove(_element, _removedIds.vids); };
		m_mesher.onElementRemoved += [this](const HMP::Dag::Element& _element, const Meshing::Mesher::RemovedIds& _removedIds) { onElementRemoved(_element, _removedIds.vids, _removedIds.vidsActuallyRemoved); };
		m_mesher.onClear += [this]() { onClearElements(); };

		onClear();
		m_commander.applied().clear();

		m_commander.applied().limit(100);
		m_commander.unapplied().limit(100);

		m_canvas.push(&m_mesh);
		m_canvas.push(&m_targetWidget.meshForDisplay());
		m_canvas.push(&m_directVertEditWidget);
		m_canvas.push(&m_axesWidget);

		m_canvas.push(&m_saveWidget);
		m_canvas.push(&m_commanderWidget);
		m_canvas.push(static_cast<cinolib::CanvasGuiItem*>(&m_vertEditWidget));
		m_canvas.push(static_cast<cinolib::SideBarItem*>(&m_vertEditWidget));
		m_canvas.push(&m_targetWidget);
		m_canvas.push(&m_projectionWidget);

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
		m_canvas.push(&m_ae3d2ShapeExporter);
#endif

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		m_canvas.push(&m_dagViewerWidget);
#endif

		m_saveWidget.onExportMesh += [this](const std::string& _filename) { onExportMesh(_filename); };
		m_saveWidget.onSave += [this](const std::string& _filename) { onSaveState(_filename); };
		m_saveWidget.onLoad += [this](const std::string& _filename) { onLoadState(_filename); };

		m_projectionWidget.onProjectRequest += [this](auto && ..._args) { onProjectToTarget(_args ...); };
		m_targetWidget.onMeshChanged += [this]() { m_canvas.refit_scene(); };
		m_targetWidget.onApplyTransformToSource += [this](const Mat4& _transform) { onApplyTargetTransform(_transform); };

		m_vertEditWidget.onApplyAction += [this](std::vector<Id> _vids, Mat4 _transform) { onApplyVertEdit(_vids, _transform); };
		m_vertEditWidget.onPendingActionChanged += [this]() { onVertEditPendingActionChanged(); };

		m_directVertEditWidget.onPendingChanged += [this]() { updateMouse(); };
		m_directVertEditWidget.color = c_overlayColor;
		m_directVertEditWidget.mutedColor = c_mutedOverlayColor;

		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_left_click = [this](auto && ..._args) { return onMouseLeftClicked(_args ...); };
		m_canvas.callback_mouse_right_click = [this](auto && ..._args) { return onMouseRightClicked(_args ...); };
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMoved(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPressed(_args...); };
		m_canvas.callback_key_event = [this](auto && ..._args) { updateMouse(); };
		m_canvas.callback_app_controls = [this](auto && ..._args) { return onDrawControls(_args ...); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChanged(_args...); };
		m_canvas.callback_custom_gui = [this](auto && ..._args) { return onDrawCustomGui(_args...); };
		m_canvas.callback_drop_files = [this](std::vector<std::string> _files) { onFilesDropped(_files); };

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		m_dagViewerWidget.onDraw += [this]() { onDagViewerDraw(); };
#endif
		requestDagViewerUpdate();
	}

	int App::launch()
	{
		m_canvas.print_key_bindings();
		printKeyBindings();
		try
		{
			return m_canvas.launch({}, false);
		}
		catch (...)
		{
			const std::time_t time{ std::time(nullptr) };
			const std::tm* now{ std::localtime(&time) };
			std::ostringstream basename{};
			basename << "crash_" << std::put_time(now, "%H-%M-%S_%d-%m-%y");
			{
				const std::string filename{ basename.str() + "_dag.hmp" };
				onSaveState(filename);
				std::cout << "Wrote state to " << std::filesystem::absolute(filename) << std::endl;
			}
			{
				const std::string filename{ basename.str() + "_mesh_debug.txt" };
				std::ofstream file;
				file.open(filename);
				file << getDebugInfo();
				file.close();
				std::cout << "Wrote mesh debug info to " << std::filesystem::absolute(filename) << std::endl;
			}
			throw;
		}
	}

}