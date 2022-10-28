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
#include <HMP/Gui/Utils/HrDescriptions.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <cpputils/collections/conversions.hpp>
#include <sstream>
#include <iomanip>

namespace HMP::Gui
{

	void App::printKeyBindings()
	{
		std::cout << "------ App key bindings -------\n";
		cinolib::print_binding(c_kbExtrude.name(), "extrude");
		cinolib::print_binding(c_kbExtrudeAndSelect.name(), "extrude and select");
		cinolib::print_binding(c_kbRefine.name(), "refine");
		cinolib::print_binding(c_kbDoubleRefine.name(), "refine twice");
		cinolib::print_binding(c_kbFaceRefine.name(), "refine face");
		cinolib::print_binding(c_kbDelete.name(), "delete");
		cinolib::print_binding(c_kbCopy.name(), "copy");
		cinolib::print_binding(c_kbPaste.name(), "paste");
		cinolib::print_binding(c_kbRotate.name(), "rotate");
		cinolib::print_binding(c_kbClear.name(), "clear");
		cinolib::print_binding(c_kbMakeConforming.name(), "make conforming");
		cinolib::print_binding(c_kbSelectVertex.name(), "select vertex");
		cinolib::print_binding(c_kbSelectEdge.name(), "select edge vertices");
		cinolib::print_binding(c_kbSelectFace.name(), "select face vertices");
		cinolib::print_binding(c_kbSelectPoly.name(), "select poly vertices");
		cinolib::print_binding(cinolib::KeyBinding::mod_names(c_kmodSelectAdd), "remove from selection (hold down)");
		cinolib::print_binding(cinolib::KeyBinding::mod_names(c_kmodSelectRemove), "add to selection (hold down)");
		cinolib::print_binding(c_kbDeselectAll.name(), "deselect all vertices");
		cinolib::print_binding(c_kbDirectTranslation.name(), "translate selected vertices");
		cinolib::print_binding(c_kbDirectScale.name(), "scale selected vertices");
		cinolib::print_binding(c_kbDirectRotation.name(), "rotate selected vertices");
		cinolib::print_binding(c_kbCancelDirectEdit.name(), "cancel direct edit");
		cinolib::print_binding(c_kbSave.name(), "save");
		cinolib::print_binding(c_kbOpen.name(), "open");
		cinolib::print_binding(c_kbSaveMesh.name(), "save mesh");
		cinolib::print_binding(c_kbLoadTarget.name(), "load target mesh");
		cinolib::print_binding(c_kbToggleTargetVisibility.name(), "toggle target visibility");
		cinolib::print_binding(c_kbUndo.name(), "undo");
		cinolib::print_binding(c_kbRedo.name(), "redo");
		cinolib::print_binding(c_kbPrintDebugInfo.name(), "print debug info");
		std::cout << "-------------------------------\n";
	}

	// markers

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
					.color = c_overlayColor,
					.shape_radius = highlightRadius,
					.shape = cinolib::Marker::EShape::CircleFilled
			};
			set[1] = cinolib::Marker{
					.pos_3d{Meshing::Utils::midpoint(m_mesh, eid)},
					.color{c_overlayColor},
					.shape_radius = highlightRadius,
					.shape = cinolib::Marker::EShape::Cross45,
					.line_thickness = 2.0f
			};
		}
		else
		{
			set.clear();
		}
	}

	void App::updateVertSelectionMarkers()
	{
		std::vector<cinolib::Marker>& set{ m_canvas.marker_sets[c_vertSelectionMarkerSetInd] };
		set.clear();
		set.reserve(m_vertEditWidget.vids().size() + 1);
		for (const Id vid : m_vertEditWidget.vids())
		{
			set.push_back(cinolib::Marker{
				.pos_3d{m_mesh.vert(vid)},
				.color{c_overlayColor},
				.shape_radius = 6u,
				.shape = cinolib::Marker::EShape::CircleOutline,
				.line_thickness = 1.5f
				});
		}
		if (!m_vertEditWidget.empty())
		{
			set.push_back(cinolib::Marker{
				.pos_3d{m_vertEditWidget.centroid()},
				.color{c_overlayColor},
				.shape_radius = 6u,
				.shape = cinolib::Marker::EShape::Cross90,
				.line_thickness = 2.0f,
				});
		}
	}

	void App::updateElementsMarkers()
	{
		std::vector<cinolib::Marker>& set{ m_canvas.marker_sets[c_elementsMarkerSetInd] };
		set.clear();
		static constexpr unsigned int nameFontSize{ 20 };
		if (m_options.showNames)
		{
			set.reserve(m_mesh.num_polys());
			for (Id pid{}; pid < m_mesh.num_polys(); pid++)
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
						.shape_radius = 0,
						.font_size = nameFontSize
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

	// actions

	void App::onActionApplied()
	{
		m_vertEditWidget.updateCentroid();
		updateMouse();
		updateAllMarkers();
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
		m_dagViewerNeedsUpdate = true;
	}

	// direct vert edit

	void App::updateDirectVertEdit()
	{
		if (!m_directVertEdit.pending)
		{
			return;
		}
		const Vec up{ m_canvas.camera.view.normUp() };
		const Vec right{ m_canvas.camera.view.normRight() };
		Utils::Transform& transform{ m_vertEditWidget.transform() };
		transform.translation = { 0.0 };
		transform.scale = { 1.0 };
		transform.rotation = { 0.0 };
		switch (m_directVertEdit.kind)
		{
			case EDirectVertEdit::Translate:
			{
			}
			break;
			case EDirectVertEdit::Rotate:
			{
				Vec2 startDir{ m_directVertEdit.startPos - m_directVertEdit.startCentroidPos };
				startDir = startDir.is_null() ? Vec2{ 1, 0 } : startDir.normalized();
				Vec2 dir{ m_mouse.position - m_directVertEdit.startCentroidPos };
				dir = dir.is_null() ? Vec2{ 1, 0 } : dir.normalized();
				const Real angle{ Utils::Transform::wrapAngle(cinolib::to_deg(std::atan2(startDir.y(), startDir.x()) - std::atan2(dir.y(), dir.x()))) };
				m_directVertEdit.transform = { angle };
				const Mat3 mat{ Utils::Transform::rotationMat(m_canvas.camera.view.normBack(), angle) };
				transform.rotation = Utils::Transform::rotationMatToVec(mat);
			}
			break;
		}
		m_vertEditWidget.applyTransform();
	}

	void App::onDirectVertEditRequested(EDirectVertEdit _kind)
	{
		const bool wasPending{ m_directVertEdit.pending };
		m_directVertEdit.pending = false;
		if (m_vertEditWidget.empty())
		{
			return;
		}
		m_vertEditWidget.applyAction();
		m_directVertEdit.pending = !wasPending || m_directVertEdit.kind != _kind;
		m_directVertEdit.startPos = m_mouse.position;
		GLdouble depth;
		m_canvas.project(m_vertEditWidget.centroid(), m_directVertEdit.startCentroidPos, depth);
		m_directVertEdit.kind = _kind;
		switch (m_directVertEdit.kind)
		{
			case EDirectVertEdit::Scale:
				m_directVertEdit.transform = Vec2{ 1.0 };
				break;
			case EDirectVertEdit::Translate:
				m_directVertEdit.transform = Vec2{};
				break;
			case EDirectVertEdit::Rotate:
				m_directVertEdit.transform = Vec2{ 0 };
				break;
		}
		updateMouse();
	}

	void App::onCancelDirectVertEdit()
	{
		if (!m_directVertEdit.pending)
		{
			return;
		}
		m_vertEditWidget.cancel();
		m_directVertEdit.pending = false;
		updateMouse();
	}

	// mesher events

	void App::onElementRemove(const HMP::Dag::Element& _element)
	{
		if (m_mouse.element == &_element)
		{
			m_mouse.element = nullptr;
		}
		std::vector<Id> vids{};
		for (const Id vid : m_mesh.poly_verts_id(m_mesher.elementToPid(_element)))
		{
			if (m_mesh.adj_v2p(vid).size() == 1)
			{
				vids.push_back(vid);
			}
		}
		m_vertEditWidget.remove(vids);
	}

	void App::onClearElements()
	{
		m_mouse.element = nullptr;
		m_vertEditWidget.clear();
	}

	// vert edit events

	void App::onVertEditVidsOrCentroidChanged()
	{
		updateVertSelectionMarkers();
	}

	void App::onVertEditMeshUpdated()
	{
		updateAllMarkers();
	}

	void App::onApplyVertEdit(const std::vector<Id>& _vids, const Mat4& _transform)
	{
		std::vector<Actions::TransformVerts::Vert> verts{};
		verts.reserve(_vids.size());
		for (const Id vid : _vids)
		{
			const Id pid{ m_mesh.adj_v2p(vid)[0] };
			const Id vertOffs{ m_mesh.poly_vert_offset(pid, vid) };
			verts.push_back({ m_mesher.pidToElement(pid), vertOffs });
		}
		applyAction(*new Actions::TransformVerts{ _transform, verts });
	}

	void App::onVertEditPendingActionChanged()
	{
		if (m_vertEditWidget.pendingAction())
		{
			m_commander.unapplied().clear();
		}
	}

	// canvas events

	void App::onCameraChanged()
	{
		onCancelDirectVertEdit();
		updateMouse();
	}

	bool App::onKeyPressed(int _key, int _modifiers)
	{
		cinolib::KeyBinding key{ _key, _modifiers };
		// extrude
		if (key == c_kbExtrude)
		{
			onExtrude();
		}
		// extrude and select
		else if (key == c_kbExtrudeAndSelect)
		{
			onExtrudeAndSelect();
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
		// direct manipulation
		else if (key == c_kbDirectTranslation)
		{
			onDirectVertEditRequested(EDirectVertEdit::Translate);
		}
		else if (key == c_kbDirectScale)
		{
			onDirectVertEditRequested(EDirectVertEdit::Scale);
		}
		else if (key == c_kbDirectRotation)
		{
			onDirectVertEditRequested(EDirectVertEdit::Rotate);
		}
		else if (key == c_kbCancelDirectEdit)
		{
			onCancelDirectVertEdit();
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
		else if (key == c_kbSelectFace)
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Set);
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
		else if (key == (c_kbSelectFace | c_kmodSelectAdd))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Add);
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
		else if (key == (c_kbSelectFace | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Face, ESelectionMode::Remove);
		}
		else if (key == (c_kbSelectPoly | c_kmodSelectRemove))
		{
			onSelect(ESelectionSource::Poly, ESelectionMode::Remove);
		}
		else if (key == c_kbDeselectAll)
		{
			onClearSelection();
		}
		else
		{
			return false;
		}
		return true;
	}

	bool App::onMouseMoved(double _x, double _y)
	{
		m_mouse.position = cinolib::vec2d{ _x, _y };
		updateMouse();
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
				<< "Hovering "
				<< Utils::HrDescriptions::name(*m_mouse.element, m_dagNamer)
				<< " ("
				<< "faces " << Utils::HrDescriptions::describeFaces(m_mouse.faceOffset, m_mouse.upFaceOffset)
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
			if (m_directVertEdit.pending)
			{
				using Utils::Controls::toImGui;
				ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
				const ImU32 startCol{ ImGui::ColorConvertFloat4ToU32(toImGui(c_directVertEditLineStartColor)) };
				const ImU32 col{ ImGui::ColorConvertFloat4ToU32(toImGui(c_directVertEditLineColor)) };
				const ImVec2 startCentroidPos{ toImGui(m_directVertEdit.startCentroidPos) };
				const ImVec2 startPos{ toImGui(m_directVertEdit.startPos) };
				const float fullLen{ static_cast<float>(std::max(m_canvas.width(), m_canvas.height()) * 2) };
				static constexpr float thickness{ 1.0f };
				switch (m_directVertEdit.kind)
				{
					case EDirectVertEdit::Rotate:
					{
						Vec2 startDir{ m_directVertEdit.startPos - m_directVertEdit.startCentroidPos };
						startDir = startDir.is_null() ? Vec2{ 1, 0 } : startDir.normalized();
						drawList.AddLine(startCentroidPos, toImGui(m_directVertEdit.startCentroidPos + startDir * fullLen), startCol, thickness);
						Vec2 dir{ m_mouse.position - m_directVertEdit.startCentroidPos };
						dir = dir.is_null() ? Vec2{ 1, 0 } : dir.normalized();
						drawList.AddLine(startCentroidPos, toImGui(m_directVertEdit.startCentroidPos + dir * fullLen), col, thickness);
						ImGui::TextColored(toImGui(c_warningTextColor), "Rotating %d %s by %3.f degrees via direct manipulation", vertexCount, verticesLit, m_directVertEdit.transform[0]);
					}
					break;
					case EDirectVertEdit::Scale:
					{
						static constexpr int segCount{ 36 };
						drawList.AddLine({ startCentroidPos.x, startCentroidPos.y + fullLen }, { startCentroidPos.x , startCentroidPos.y - fullLen }, startCol, thickness);
						drawList.AddLine({ startCentroidPos.x + fullLen, startCentroidPos.y }, { startCentroidPos.x - fullLen, startCentroidPos.y }, startCol, thickness);
						drawList.AddLine({ startCentroidPos.x + fullLen, startCentroidPos.y + fullLen }, { startCentroidPos.x - fullLen, startCentroidPos.y - fullLen }, startCol, thickness);
						drawList.AddLine({ startCentroidPos.x + fullLen, startCentroidPos.y - fullLen }, { startCentroidPos.x - fullLen, startCentroidPos.y + fullLen }, startCol, thickness);
						drawList.AddCircle(startCentroidPos, static_cast<float>(m_directVertEdit.startCentroidPos.dist(m_directVertEdit.startPos)), startCol, segCount, thickness);
						drawList.AddCircle(startCentroidPos, static_cast<float>(m_directVertEdit.startCentroidPos.dist(m_mouse.position)), col, segCount, thickness);
						ImGui::TextColored(toImGui(c_warningTextColor), "Scaling %d %s by %2.f%%,%2.f%% via direct manipulation", vertexCount, verticesLit, m_directVertEdit.transform.x() * 100.0, m_directVertEdit.transform.y() * 100.0);
					}
					break;
					case EDirectVertEdit::Translate:
					{
						drawList.AddLine({ startPos.x, startPos.y + fullLen }, { startPos.x , startPos.y - fullLen }, startCol, thickness);
						drawList.AddLine({ startPos.x + fullLen, startPos.y }, { startPos.x - fullLen, startPos.y }, startCol, thickness);
						drawList.AddLine(startPos, toImGui(m_mouse.position), col, thickness);
						ImGui::TextColored(toImGui(c_warningTextColor), "Translating %d %s by %3.f,%3.f via direct manipulation", vertexCount, verticesLit, m_directVertEdit.transform.x(), m_directVertEdit.transform.y());
					}
					break;
				}
			}
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

	void App::updateMouse()
	{
		HMP::Dag::Element* const lastElement{ m_mouse.element };
		const Id lastFaceOffset{ m_mouse.faceOffset }, lastUpFaceOffset{ m_mouse.upFaceOffset }, lastVertOffset{ m_mouse.vertOffset };
		m_mouse.element = nullptr;
		m_mouse.faceOffset = m_mouse.vertOffset = noId;
		if (!m_directVertEdit.pending)
		{
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
		updateDirectVertEdit();
	}

	// Commands

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
				<< " centroid: " << Utils::HrDescriptions::describe(centroid)
				<< " vids: " << Utils::HrDescriptions::describe(m_mesh.adj_p2v(pid))
				<< " eids: " << Utils::HrDescriptions::describe(m_mesh.adj_p2e(pid))
				<< " fids: " << Utils::HrDescriptions::describe(m_mesh.adj_p2f(pid))
				<< " pids: " << Utils::HrDescriptions::describe(m_mesh.adj_p2p(pid))
				<< " winding: " << Utils::HrDescriptions::describe(m_mesh.poly_faces_winding(pid))
				<< " locs: " << Utils::HrDescriptions::describe(locs)
				<< "\n";
		}
		std::cout << "---- Faces\n";
		for (Id fid{}; fid < m_mesh.num_faces(); fid++)
		{
			std::cout
				<< "fid: " << fid
				<< " centroid: " << Utils::HrDescriptions::describe(m_mesh.face_centroid(fid))
				<< " vids: " << Utils::HrDescriptions::describe(m_mesh.adj_f2v(fid))
				<< " eids: " << Utils::HrDescriptions::describe(m_mesh.adj_f2e(fid))
				<< " fids: " << Utils::HrDescriptions::describe(m_mesh.adj_f2f(fid))
				<< " pids: " << Utils::HrDescriptions::describe(m_mesh.adj_f2p(fid))
				<< " normal: " << Utils::HrDescriptions::describe(m_mesh.face_data(fid).normal)
				<< "\n";
		}
		std::cout << "---- Edges\n";
		for (Id eid{}; eid < m_mesh.num_edges(); eid++)
		{
			std::cout
				<< "eid: " << eid
				<< " midpoint: " << Utils::HrDescriptions::describe(Meshing::Utils::midpoint(m_mesh, eid))
				<< " vids: " << Utils::HrDescriptions::describe(m_mesh.adj_e2v(eid))
				<< " eids: " << Utils::HrDescriptions::describe(m_mesh.adj_e2e(eid))
				<< " fids: " << Utils::HrDescriptions::describe(m_mesh.adj_e2f(eid))
				<< " pids: " << Utils::HrDescriptions::describe(m_mesh.adj_e2p(eid))
				<< "\n";
		}
		std::cout << "---- Vertices\n";
		for (Id vid{}; vid < m_mesh.num_verts(); vid++)
		{
			std::cout
				<< "vid: " << vid
				<< " position: " << Utils::HrDescriptions::describe(m_mesh.vert(vid))
				<< " vids: " << Utils::HrDescriptions::describe(m_mesh.adj_v2v(vid))
				<< " eids: " << Utils::HrDescriptions::describe(m_mesh.adj_v2e(vid))
				<< " fids: " << Utils::HrDescriptions::describe(m_mesh.adj_v2f(vid))
				<< " pids: " << Utils::HrDescriptions::describe(m_mesh.adj_v2p(vid))
				<< "\n";
		}
		std::cout << "----------------------------------" << std::endl;
	}

	void App::onExtrude()
	{
		if (m_mouse.element)
		{
			applyAction(*new Actions::Extrude{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset });
		}
	}

	void App::onExtrudeAndSelect()
	{
		if (m_vertEditWidget.vids().size() == 4)
		{

			const std::vector<Id> vids{ cpputils::collections::conversions::toVector(m_vertEditWidget.vids()) };
			const Id fid{ static_cast<Id>(m_mesh.face_id(vids)) };
			if (fid == noId)
			{
				return;
			}
			Id pid;
			if (m_mesh.face_is_visible(fid, pid))
			{
				HMP::Dag::Element& element{ m_mesher.pidToElement(pid) };
				const Id faceOffset{ m_mesh.poly_face_offset(pid, fid) };
				Id upFaceOffset{ faceOffset };
				{
					Real upFaceCentroidY{};
					for (Id fo{}; fo < 6; fo++)
					{
						if (fo == upFaceOffset)
						{
							continue;
						}
						const Real centroidY{ m_mesh.face_centroid(m_mesh.poly_face_id(pid, fo)).y() };
						if (upFaceOffset == faceOffset || centroidY > upFaceCentroidY)
						{
							upFaceCentroidY = centroidY;
							upFaceOffset = fo;
						}
					}
				}
				Actions::Extrude& action{ *new Actions::Extrude{ element, faceOffset, upFaceOffset } };
				applyAction(action);
				const HMP::Dag::Element& newElement{ action.operation().children().single() };
				const Id newPid{ m_mesher.elementToPid(newElement) };
				const Id newFid{ m_mesh.poly_face_opposite_to(newPid, fid) };
				m_vertEditWidget.clear();
				m_vertEditWidget.add(m_mesh.face_verts_id(newFid));
			}
		}
	}

	void App::onCopy()
	{
		m_copy.element = m_mouse.element;
	}

	void App::onPaste()
	{
		if (m_mouse.element && m_copy.element && m_mesher.has(*m_copy.element))
		{
			if (m_copy.element->parents().size() == 1 && m_copy.element->parents().single().primitive() == HMP::Dag::Operation::EPrimitive::Extrude)
			{
				applyAction(*new Actions::Paste{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset, static_cast<HMP::Dag::Extrude&>(m_copy.element->parents().single()) });
			}
		}
	}

	void App::onRefineElement(bool _twice)
	{
		if (m_mouse.element)
		{
			applyAction(*new Actions::Refine{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset, Meshing::ERefinementScheme::Subdivide3x3, _twice ? 2u : 1u });
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

	void App::onRotate()
	{
		if (m_mouse.element && m_mouse.element->parents().size() == 1 && m_mouse.element->parents().single().primitive() == HMP::Dag::Operation::EPrimitive::Extrude)
		{
			applyAction(*new Actions::Rotate{ static_cast<HMP::Dag::Extrude&>(m_mouse.element->parents().single()) });
		}
	}

	void App::onRefineFace()
	{
		if (m_mouse.element)
		{
			applyAction(*new Actions::Refine{ *m_mouse.element, m_mouse.faceOffset, m_mouse.upFaceOffset, Meshing::ERefinementScheme::Inset });
		}
	}

	void App::onMakeConformant()
	{
		applyAction(*new Actions::MakeConforming());
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
			HMP::Utils::Serialization::Serializer serializer{ file };
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
			HMP::Utils::Serialization::Deserializer deserializer{ file };
			HMP::Dag::Element& root = HMP::Dag::Utils::deserialize(deserializer).element();
			file.close();
			applyAction(*new Actions::Load{ root });
			m_canvas.reset_camera();
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
		applyAction(*new Actions::Project{ std::move(target) });
	}

	void App::onApplyTargetTransform(const Mat4& _transform)
	{
		applyAction(*new Actions::TransformAll{ _transform });
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
		applyAction(*new Actions::Clear());
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

	void App::onClearSelection()
	{
		m_vertEditWidget.clear();
	}

	// launch

	App::App() :
		m_project{}, m_canvas{}, m_mesher{ m_project.mesher() }, m_mesh{ m_mesher.mesh() }, m_commander{ m_project.commander() },
		m_dagNamer{}, m_dagViewer{ m_mesher, m_dagNamer }, m_menu{ const_cast<Meshing::Mesher::Mesh*>(&m_mesh), &m_canvas, "Mesh controls" },
		m_commanderWidget{ m_commander, m_dagNamer, m_vertEditWidget }, m_axesWidget{ m_canvas.camera }, m_targetWidget{ m_mesh }, m_vertEditWidget{ m_mesher }, m_dagViewerNeedsUpdate{ true }
	{

		m_canvas.background = c_backgroundColor;
		m_mesher.polyMarkerSet().color() = c_selectedPolyColor;
		m_mesher.faceMarkerSet().color() = c_selectedFaceColor;

		m_mesher.onElementRemove += [this](const HMP::Dag::Element& _element) { onElementRemove(_element); };
		m_mesher.onClear += [this]() { onClearElements(); };

		m_commander.apply(*new Actions::Clear());
		m_commander.applied().clear();

		m_commander.applied().limit(100);
		m_commander.unapplied().limit(100);

		m_canvas.push(&m_mesh);
		m_canvas.push(&m_axesWidget);

		m_canvas.push(&m_commanderWidget);
		m_canvas.push(&m_vertEditWidget);
		m_canvas.push(&m_targetWidget);
		m_canvas.push(&m_targetWidget.projectLines());
		m_canvas.push(&m_menu);
		m_canvas.push(&m_dagViewer);

		m_targetWidget.onProjectRequest += [this]() { onProjectToTarget(); };
		m_targetWidget.onMeshLoad += [this]() { m_canvas.push(&m_targetWidget.mesh(), false); };
		m_targetWidget.onMeshClear += [this]() { m_canvas.pop(&m_targetWidget.mesh()); };
		m_targetWidget.onTransform += [this]() { m_canvas.refit_scene(); };
		m_targetWidget.onApplyTransformToSource += [this](const Mat4& _transform) { onApplyTargetTransform(_transform); };

		m_vertEditWidget.onApplyAction += [this](std::vector<Id> _vids, Mat4 _transform) { onApplyVertEdit(_vids, _transform); };
		m_vertEditWidget.onMeshUpdated += [this]() { onVertEditMeshUpdated(); };
		m_vertEditWidget.onVidsChanged += [this]() { onVertEditVidsOrCentroidChanged(); };
		m_vertEditWidget.onCentroidChanged += [this]() { onVertEditVidsOrCentroidChanged(); };
		m_vertEditWidget.onPendingActionChanged += [this]() { onVertEditPendingActionChanged(); };

		m_canvas.depth_cull_markers = false;
		m_canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMoved(_args...); };
		m_canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPressed(_args...); };
		m_canvas.callback_app_controls = [this](auto && ..._args) { return onDrawControls(_args ...); };
		m_canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChanged(_args...); };
		m_canvas.callback_custom_gui = [this](auto && ..._args) { return onDrawCustomGui(_args...); };
		m_canvas.marker_sets.resize(3);

		m_dagViewer.onDraw += [this]() { onDagViewerDraw(); };
		requestDagViewerUpdate();
	}

	int App::launch()
	{
		printKeyBindings();
		return m_canvas.launch();
	}

}