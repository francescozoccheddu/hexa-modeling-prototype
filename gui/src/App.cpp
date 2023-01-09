#include <HMP/Gui/App.hpp>
#include <cinolib/gl/volume_mesh_controls.h>
#include <cinolib/gl/glcanvas.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <HMP/Dag/Utils.hpp>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <HMP/Gui/Utils/FilePicking.hpp>
#include <utility>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Actions/Root.hpp>
#include <HMP/Actions/Pad.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Project.hpp>
#include <HMP/Actions/Refine.hpp>
#include <HMP/Actions/Paste.hpp>
#include <HMP/Actions/Transform.hpp>
#include <HMP/Actions/Smooth.hpp>
#include <HMP/Actions/SubdivideAll.hpp>
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
#include <HMP/Gui/Utils/Theme.hpp>
#include <HMP/Gui/themer.hpp>
#include <HMP/Gui/Widgets/Axes.hpp>
#include <HMP/Gui/Widgets/Commander.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/Widgets/DirectVertEdit.hpp>
#include <HMP/Gui/Widgets/Projection.hpp>
#include <HMP/Gui/Widgets/Save.hpp>
#include <HMP/Gui/Widgets/Debug.hpp>
#include <HMP/Gui/Widgets/Pad.hpp>
#include <HMP/Gui/Widgets/Smooth.hpp>

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
#include <HMP/Gui/DagViewer/Widget.hpp>
#endif

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
#include <HMP/Gui/Widgets/Ae3d2ShapeExporter.hpp>
#endif


#ifdef HMP_GUI_ENABLE_DAG_VIEWER
#include <HMP/Gui/DagViewer/createLayout.hpp>
#endif

namespace HMP::Gui
{

	void App::printUsage() const
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
		cinolib::print_binding(c_kbSubdivideAll.name(), "subdivide all");
		cinolib::print_binding(c_kbMakeConforming.name(), "make conforming");
		cinolib::print_binding(c_kbUndo.name(), "undo");
		cinolib::print_binding(c_kbRedo.name(), "redo");
		cinolib::print_binding(c_kbRedo.name(), "redo");
		for (Widget* const widget : m_widgets)
		{
			widget->printUsage();
		}
		std::cout << "-------------------------------\n";
	}

	// actions

	void App::onActionApplied()
	{
		mesher.updateMesh();
		vertEditWidget.updateCentroid();
		m_mouse.element = nullptr;
		updateMouse();
		requestDagViewerUpdate();
		canvas.refit_scene();
	}

	void App::applyAction(Commander::Action& _action)
	{
		vertEditWidget.applyAction();
		commander.apply(_action);
		onActionApplied();
	}

	void App::requestDagViewerUpdate()
	{
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		m_dagViewerNeedsUpdate = true;
#endif
	}

	// mesher events

	void App::onMesherRestored(const Meshing::Mesher::State&)
	{
		if (m_mouse.element && m_mouse.element->pid >= mesher.mesh().num_polys())
		{
			m_mouse.element = nullptr;
		}
		if (m_copy.element && m_copy.element->pid >= mesher.mesh().num_polys())
		{
			m_copy.element = nullptr;
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
			dagViewerWidget.copied = nullptr;
#endif
		}
		vertEditWidget.remove(vertEditWidget.vids().filter([&](const Id _vid) {
			return _vid >= mesher.mesh().num_verts();
		}).toVector());
	}

	void App::onMesherElementVisibilityChanged(const Dag::Element& _element, bool _visible)
	{
		if (!_visible)
		{
			vertEditWidget.remove(mesher.mesh().poly_dangling_vids(_element.pid));
		}
	}

	// vert edit events

	void App::onApplyVertEdit(const std::vector<Id>& _vids, const Mat4& _transform)
	{
		applyAction(*new Actions::Transform{ _transform, _vids });
	}

	void App::onVertEditPendingActionChanged()
	{
		if (vertEditWidget.pendingAction())
		{
			commander.unapplied().clear();
		}
	}

	// save events

	void App::onExportMesh(const std::string& _filename) const
	{
		Meshing::Mesher::Mesh mesh{ mesher.mesh() };
		for (Id pidPlusOne{ mesh.num_polys() }; pidPlusOne > 0; --pidPlusOne)
		{
			if (!mesher.shown(pidPlusOne - 1))
			{
				mesh.poly_remove(pidPlusOne - 1, true);
			}
		}
		mesh.save(_filename.c_str());
	}

	void App::onSaveState(const std::string& _filename)
	{
		std::ofstream file;
		file.open(_filename);
		HMP::Utils::Serialization::Serializer serializer{ file };
		HMP::Dag::Utils::serialize(serializer, *project.root());
		serializer << toI(mesher.mesh().num_verts());
		for (const Vec& vert : mesher.mesh().vector_verts())
		{
			serializer << vert;
		}
		for (Widget* const widget : m_widgets)
		{
			widget->serialize(serializer);
		}
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
		applyAction(*new Actions::Root{ root, verts });
		canvas.reset_camera();
		for (Widget* const widget : m_widgets)
		{
			widget->deserialize(deserializer);
		}
		file.close();
	}

	// canvas events

	void App::onCameraChanged()
	{
		for (Widget* const widget : m_widgets)
		{
			widget->cameraChanged();
		}
		updateMouse();
	}

	bool App::onMouseLeftClicked(int)
	{
		for (Widget* const widget : m_widgets)
		{
			if (widget->mouseClicked(false))
			{
				return true;
			}
		}
		return false;
	}

	bool App::onMouseRightClicked(int)
	{
		for (Widget* const widget : m_widgets)
		{
			if (widget->mouseClicked(true))
			{
				return true;
			}
		}
		return false;
	}

	bool App::onKeyPressed(int _key, int _modifiers)
	{
		cinolib::KeyBinding key{ _key, _modifiers };
		for (Widget* const widget : m_widgets)
		{
			if (widget->keyPressed(key))
			{
				return true;
			}
		}
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
		// subdivide all
		else if (key == c_kbSubdivideAll)
		{
			onSubdivideAll();
		}
		else
		{
			return false;
		}
		return true;
	}

	bool App::onMouseMoved(double _x, double _y)
	{
		for (Widget* const widget : m_widgets)
		{
			widget->mouseMoved({_x, _y});
		}
		m_mouse.position = cinolib::vec2d{ _x, _y };
		updateMouse();
		return directVertEditWidget.pending();
	}

	void App::onDrawCustomGui()
	{
		const float
			smallVertRadius{ 4.0f * themer->ovScale },
			vertRadius{ 6.0f * themer->ovScale },
			lineThickness{ 1.5f * themer->ovScale },
			semiBoldLineThickness{ 2.5f * themer->ovScale },
			boldLineThickness{ 3.5f * themer->ovScale },
			lineSpacing{ (10.0f * themer->ovScale + 10.0f) / 2.0f };
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		using namespace Utils::Drawing;
		if (m_copy.element && !m_mouse.element)
		{
			const Id cPid{ m_copy.element->pid };
			const Dag::Extrude& extrude{ m_copy.element->parents.single().as<Dag::Extrude>() };
			const auto cPidCenter2d{ Utils::Drawing::project(canvas, mesher.mesh().poly_centroid(cPid)) };
			if (cPidCenter2d)
			{
				for (const auto& [parent, fi] : extrude.parents.zip(extrude.fis))
				{
					const QuadVertIds parentFidVids{ Meshing::Utils::fiVids(parent.vids, fi) };
					const Vec parentFidCenter{ Meshing::Utils::centroid(Meshing::Utils::verts(mesher.mesh(), parentFidVids)) };
					const auto parentFidCenter2d{ Utils::Drawing::project(canvas, parentFidCenter) };
					if (parentFidCenter2d)
					{
						dashedLine(drawList, { *parentFidCenter2d, *cPidCenter2d }, themer->ovMut, lineThickness, lineSpacing);
					}
				}
			}
			circle(drawList, cPidCenter2d, smallVertRadius, m_mouse.element == m_copy.element ? themer->ovHi : themer->ovMut, lineThickness);
			const Dag::Element& firstParent{ extrude.parents.first() };
			const Id firstVid{ firstParent.vids[extrude.firstVi] };
			const QuadVertIds firstParentVids{ Meshing::Utils::align(Meshing::Utils::fiVids(firstParent.vids, extrude.fis[0]), firstVid, extrude.clockwise) };
			const auto eid2d{ Utils::Drawing::project(canvas, Meshing::Utils::verts(mesher.mesh(), EdgeVertIds{ firstParentVids[0], firstParentVids[1] })) };
			dashedLine(drawList, eid2d, themer->ovMut, boldLineThickness, lineSpacing);
			if (eid2d)
			{
				circleFilled(drawList, (*eid2d)[0], vertRadius, themer->ovMut);
			}
		}
		if (m_mouse.element)
		{
			for (I i{}; i < 6; i++)
			{
				const I fi{ (i + 1 + m_mouse.fi) % 6 };
				const QuadVerts fiVerts{ Meshing::Utils::verts(mesher.mesh(), Meshing::Utils::fiVids(m_mouse.element->vids, fi)) };
				const auto fiVerts2d{ Utils::Drawing::project(canvas, fiVerts) };
				quadFilled(drawList, fiVerts2d, fi == m_mouse.fi ? themer->ovFaceHi : themer->ovPolyHi);
			}
			const auto hPidCenter2d{ Utils::Drawing::project(canvas, mesher.mesh().poly_centroid(m_mouse.pid)) };
			if (hPidCenter2d)
			{
				for (const Id adjPid : mesher.mesh().adj_p2p(m_mouse.pid))
				{
					const Id adjFid{ static_cast<Id>(mesher.mesh().poly_shared_face(m_mouse.pid, adjPid)) };
					const auto adjFidCenter2d{ Utils::Drawing::project(canvas, mesher.mesh().face_centroid(adjFid)) };
					const auto adjPidCenter2d{ Utils::Drawing::project(canvas, mesher.mesh().poly_centroid(adjPid)) };
					const ImU32 adjColorU32{ mesher.shown(adjPid) ? themer->ovHi : themer->ovMut };
					circle(drawList, adjPidCenter2d, smallVertRadius, adjColorU32, lineThickness);
					if (adjFidCenter2d)
					{
						dashedLine(drawList, { *adjFidCenter2d, *hPidCenter2d }, adjColorU32, lineThickness, lineSpacing);
					}
				}
				circle(drawList, *hPidCenter2d, smallVertRadius, themer->ovHi, lineThickness);
			}
			for (const Id adjEid : mesher.mesh().adj_f2e(m_mouse.fid))
			{
				const auto adjEid2d{ Utils::Drawing::project(canvas, Meshing::Utils::verts(mesher.mesh(), Meshing::Utils::eidVids(mesher.mesh(), adjEid))) };
				dashedLine(drawList, adjEid2d, adjEid == m_mouse.eid ? themer->ovHi : themer->ovMut, boldLineThickness, lineSpacing);
			}
			const auto hVert2d{ Utils::Drawing::project(canvas, mesher.mesh().vert(m_mouse.vid)) };
			circleFilled(drawList, hVert2d, vertRadius, themer->ovHi);
		}
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		if (canvas.show_sidebar() && static_cast<const cinolib::SideBarItem&>(dagViewerWidget).show_open)
		{
			if (dagViewerWidget.hasHoveredNode() && dagViewerWidget.hoveredNode().isElement())
			{
				const Id pid{ dagViewerWidget.hoveredNode().element().pid };
				for (const Id fid : mesher.mesh().adj_p2f(pid))
				{
					const QuadVerts fidVerts{ Meshing::Utils::verts(mesher.mesh(), Meshing::Utils::fidVids(mesher.mesh(), fid)) };
					const auto fidVerts2d{ Utils::Drawing::project(canvas, fidVerts) };
					quadFilled(drawList, fidVerts2d, themer->ovPolyHi);
				}
				for (const Id eid : mesher.mesh().adj_p2e(pid))
				{
					const EdgeVerts eidVerts{ Meshing::Utils::verts(mesher.mesh(), Meshing::Utils::eidVids(mesher.mesh(), eid)) };
					const auto eidVerts2d{ Utils::Drawing::project(canvas, eidVerts) };
					dashedLine(drawList, eidVerts2d, themer->ovFaceHi, semiBoldLineThickness, lineSpacing);
				}
			}
		}
		else
		{
			dagViewerWidget.showLayoutPerformanceWarning = false;
		}
#endif
		if (m_mouse.element)
		{
			std::ostringstream stream{};
			stream
				<< "Hovering "
				<< Utils::HrDescriptions::name(*m_mouse.element, dagNamer)
				<< " ("
				<< "face " << m_mouse.fi
				<< ", edge " << m_mouse.ei
				<< ", vert " << m_mouse.vi
				<< ")";
			ImGui::TextColored(toImVec4(themer->ovMut), "%s", stream.str().c_str());
		}
		if (m_copy.element)
		{
			std::ostringstream stream{};
			stream
				<< "Copied"
				<< " " << Utils::HrDescriptions::name(*m_copy.element, dagNamer);
			ImGui::TextColored(toImVec4(themer->ovMut), "%s", stream.str().c_str());
		}
		if (!vertEditWidget.empty())
		{
			const char* verticesLit{ vertEditWidget.vids().size() == 1 ? "vertex" : "vertices" };
			const int vertexCount{ static_cast<int>(vertEditWidget.vids().size()) };
			ImGui::TextColored(toImVec4(themer->ovMut), "%d %s selected", vertexCount, verticesLit);
			
		}
	}

	void App::onDagViewerDraw()
	{
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		if (m_dagViewerNeedsUpdate)
		{
			m_dagViewerNeedsUpdate = false;
			if (project.root() && mesher.mesh().num_polys() < 100000)
			{
				dagViewerWidget.tooManyNodes = false;
				dagViewerWidget.layout() = DagViewer::createLayout(*project.root());
			}
			else
			{
				dagViewerWidget.tooManyNodes = true;
			}
			dagViewerWidget.resetView();
		}
#endif
	}

	void App::updateMouse()
	{
		m_mouse.element = nullptr;
		if (!directVertEditWidget.pending())
		{
			const cinolib::Ray ray{ canvas.eye_to_mouse_ray() };
			if (mesher.pick(ray.begin(), ray.dir(), m_mouse.pid, m_mouse.fid, m_mouse.eid, m_mouse.vid, !canvas.camera.projection.perspective))
			{
				m_mouse.element = &mesher.element(m_mouse.pid);
				m_mouse.fi = Meshing::Utils::fi(m_mouse.element->vids, Meshing::Utils::fidVids(mesher.mesh(), m_mouse.fid));
				m_mouse.ei = Meshing::Utils::ei(m_mouse.element->vids, Meshing::Utils::eidVids(mesher.mesh(), m_mouse.eid));
				m_mouse.vi = Meshing::Utils::vi(m_mouse.element->vids, m_mouse.vid);
			}
		}
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		dagViewerWidget.highlight = m_mouse.element;
#endif
	}

	// Commands

	bool App::hoveredExtrudeElements(Dag::Extrude::ESource _source, cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, cpputils::collections::FixedVector<I, 3>& _fis, I& _firstVi, bool& _clockwise)
	{
		cpputils::collections::FixedVector<Id, 3> pids, fids;
		if (m_mouse.element)
		{
			pids.addLast(m_mouse.pid);
			fids.addLast(m_mouse.fid);
			const Id commVid{ m_mouse.vid };
			const Id firstEid{ m_mouse.eid };
			if (_source != Dag::Extrude::ESource::Face)
			{
				const cinolib::Plane firstPlane{
					mesher.mesh().face_centroid(fids[0]),
					mesher.mesh().poly_face_normal(pids[0], fids[0])
				};
				for (const Id adjFid : mesher.mesh().adj_e2f(firstEid))
				{
					Id adjPid;
					if (mesher.mesh().face_is_visible(adjFid, adjPid)
						&& adjPid != pids[0]
						&& firstPlane.point_plane_dist_signed(mesher.mesh().face_centroid(adjFid)) > 0)
					{
						if (fids.size() == 2)
						{
							const auto edgeVec{ [&](const Id _pid, const Id _fid) {
								return mesher.mesh().edge_vec(static_cast<Id>(mesher.mesh().edge_id(commVid, mesher.mesh().poly_vert_opposite_to(_pid, _fid, commVid))), true);
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
					for (const Id adjFid : mesher.mesh().adj_f2f(fids[0]))
					{
						Id adjPid;
						if (mesher.mesh().face_is_visible(adjFid, adjPid)
							&& adjPid != pids[0] && adjPid != pids[1]
							&& adjFid != fids[0] && adjFid != fids[1]
							&& mesher.mesh().face_contains_vert(adjFid, commVid)
							&& mesher.mesh().faces_are_adjacent(adjFid, fids[1]))
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
			_clockwise = Meshing::Utils::isEdgeCW(mesher.mesh(), pids[0], fids[0], commVid, firstEid);
			_elements = cpputils::range::of(pids).map([&](Id _pid) {
				return &mesher.element(_pid);
			}).toFixedVector<3>();
			_fis = cpputils::range::zip(fids, _elements).map([&](const auto& _fidAndElement) {
				const auto& [fid, element] {_fidAndElement};
			const QuadVertIds vids{ Meshing::Utils::fidVids(mesher.mesh(), fid) };
			return Meshing::Utils::fi(element->vids, vids);
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
		const std::vector<Id> vids{ vertEditWidget.vids().toVector() };
		if (vids.size() != 4)
		{
			return;
		}
		const Id fid{ static_cast<Id>(mesher.mesh().face_id(vids)) };
		if (fid == noId)
		{
			return;
		}
		Id pid;
		if (!mesher.mesh().face_is_visible(fid, pid))
		{
			return;
		}
		Dag::Element& element{ mesher.element(pid) };
		const I fi{ Meshing::Utils::fi(element.vids, Meshing::Utils::fidVids(mesher.mesh(), fid)) };
		const I vi{ Meshing::Utils::vi(element.vids, vids[0]) };
		Actions::Extrude& action{ *new Actions::Extrude{ {&element}, {fi}, vi, false } };
		applyAction(action);
		const Id newPid{ action.operation().children.single().pid };
		const Id newFid{ mesher.mesh().poly_face_opposite_to(newPid, fid) };
		vertEditWidget.clear();
		vertEditWidget.add(mesher.mesh().face_verts_id(newFid));
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
		dagViewerWidget.copied = m_copy.element;
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
				applyAction(*new Actions::Paste{ elements, fis, firstVi, clockwise, m_copy.element->parents.single().as<Dag::Extrude>() });
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

	void App::onRefineTest(Refinement::EScheme _scheme, I _forwardFi, I _firstVi)
	{
		if (mesher.mesh().num_polys() == 1)
		{
			applyAction(*new Actions::Refine{ mesher.element(0), _forwardFi, _firstVi, _scheme });
		}
	}

	void App::onSubdivideAll()
	{
		applyAction(*new Actions::SubdivideAll{});
	}

	void App::onDelete()
	{
		if (mesher.mesh().num_polys() <= 1)
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

	void App::onToggleTargetVisibility()
	{
		if (targetWidget.hasMesh())
		{
			targetWidget.visible ^= true;
			targetWidget.updateVisibility();
		}
	}

	void App::onProjectToTarget(const cinolib::Polygonmesh<>& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options)
	{
		applyAction(*new Actions::Project{ _target, _pointFeats, _pathFeats, _options });
	}

	void App::onApplyTargetTransform(const Mat4& _transform)
	{
		applyAction(*new Actions::Transform{ _transform });
		canvas.reset_camera();
	}

	void App::onUndo()
	{
		if (commander.canUndo())
		{
			vertEditWidget.applyAction();
			commander.undo();
			onActionApplied();
		}
		else
		{
			std::cout << "cannot undo" << std::endl;
		}
	}

	void App::onRedo()
	{
		if (commander.canRedo())
		{
			vertEditWidget.applyAction();
			commander.redo();
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
		for (char& c : ext) c = static_cast<char>(std::tolower(c));
		if (projectExts.contains(ext))
		{
			saveWidget.requestLoad(_file);
		}
		else if (targetMeshExts.contains(ext))
		{
			targetWidget.load(_file);
		}
		else
		{
			std::cerr << "unknown extension '" << ext << "'" << std::endl;
			std::cout << "only *.off, *.obj and *.stl target mesh files and *.hmp project files are supported" << std::endl;
		}
	}

	void App::onThemeChanged()
	{
		canvas.background = themer->bg;
		mesher.edgeColor = themer->srcEdge;
		mesher.faceColor = themer->srcFace;
		mesher.setEdgeThickness(2.0f * themer->ovScale);
		mesher.updateColors();
	}

	void App::onPad(Real _length, I _smoothIterations, Real _smoothSurfVertWeight, Real _cornerShrinkFactor)
	{
		applyAction(*new Actions::Pad{ _length, _smoothIterations, _smoothSurfVertWeight, _cornerShrinkFactor });
	}

	void App::onSmooth(I _surfaceIterations, I _internalIterations, Real _surfVertWeight)
	{
		applyAction(*new Actions::Smooth{ _surfaceIterations, _internalIterations, _surfVertWeight });
	}

	// launch

	App::App():
		project{}, canvas{ 700, 600, 13, 1.0f }, mesher{ project.mesher() }, commander{ project.commander() }, dagNamer{}, 
		commanderWidget{ *new Widgets::Commander{ commander, dagNamer, vertEditWidget } },
		axesWidget{ *new Widgets::Axes{} },
		targetWidget{ *new Widgets::Target{mesher.mesh() } },
		vertEditWidget{ *new Widgets::VertEdit{mesher } },
		directVertEditWidget{ *new Widgets::DirectVertEdit{vertEditWidget, canvas } },
		saveWidget{ *new Widgets::Save{} },
		projectionWidget{ *new Widgets::Projection{ targetWidget, commander, mesher } },
		debugWidget{ *new Widgets::Debug{mesher, dagNamer, vertEditWidget, targetWidget } },
		padWidget{ *new Widgets::Pad{ mesher.mesh() } },
		smoothWidget{ *new Widgets::Smooth{} },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		dagViewerWidget{ *new DagViewer::Widget{ dagNamer } },
		m_dagViewerNeedsUpdate{ true },
#endif
#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
		ae3d2ShapeExporter{ *new Widgets::Ae3d2ShapeExporter{ mesher.mesh(), canvas.camera, targetWidget } },
#endif
		m_widgets {
			&debugWidget, 
			&saveWidget, 
			&commanderWidget, 
			&axesWidget, 
			&vertEditWidget, 
			&directVertEditWidget, 
			&padWidget, 
			&smoothWidget,
			&targetWidget, 
			&projectionWidget, 
#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
			&ae3d2ShapeExporter,
#endif
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
			&dagViewerWidget,
#endif
		}
	{

#ifdef NDEBUG
#define HMP_GUI_APP_TITLE HMP_NAME " v" HMP_VERSION
#else
#define HMP_GUI_APP_TITLE HMP_NAME " v" HMP_VERSION " (DEV)"
#endif
		glfwSetWindowTitle(canvas.window, HMP_GUI_APP_TITLE);
#undef HMP_GUI_APP_TITLE

		canvas.key_bindings.reset_camera = GLFW_KEY_P;
		canvas.key_bindings.store_camera = { GLFW_KEY_C, GLFW_MOD_ALT };
		canvas.key_bindings.restore_camera = { GLFW_KEY_V, GLFW_MOD_ALT };

		mesher.onRestored += [this](const Meshing::Mesher::State& _oldState) { onMesherRestored(_oldState); };
		mesher.onElementVisibilityChanged += [this](const Dag::Element& _element, bool _visible) { onMesherElementVisibilityChanged(_element, _visible); };

		onClear();
		commander.applied().clear();

		commander.applied().limit(100);
		commander.unapplied().limit(100);

		for (Widget* widget : m_widgets)
		{
			widget->m_app = this;
			canvas.push(static_cast<cinolib::CanvasGuiItem*>(widget));
			if (SidebarWidget* const sidebarWidget{ dynamic_cast<SidebarWidget*>(widget) })
			{
				canvas.push(static_cast<cinolib::SideBarItem*>(sidebarWidget));
			}
			for (const cinolib::DrawableObject* additionalDrawable : widget->additionalDrawables())
			{
				canvas.push(additionalDrawable);
			}
		}
		for (Widget* widget : m_widgets)
		{
			widget->attached();
		}
		canvas.push(&mesher.mesh());

		padWidget.onPadRequested += [this](const auto&& ... _args) { onPad(_args...); };
		smoothWidget.onSmoothRequested += [this](const auto&& ... _args) { onSmooth(_args...); };

		saveWidget.onExportMesh += [this](const std::string& _filename) { onExportMesh(_filename); };
		saveWidget.onSave += [this](const std::string& _filename) { onSaveState(_filename); };
		saveWidget.onLoad += [this](const std::string& _filename) { onLoadState(_filename); };

		projectionWidget.onProjectRequest += [this](auto && ..._args) { onProjectToTarget(_args ...); };

		targetWidget.onMeshShapeChanged += [this]() { canvas.refit_scene(); };
		targetWidget.onApplyTransformToSource += [this](const Mat4& _transform) { onApplyTargetTransform(_transform); };

		vertEditWidget.onApplyAction += [this](std::vector<Id> _vids, Mat4 _transform) { onApplyVertEdit(_vids, _transform); };
		vertEditWidget.onPendingActionChanged += [this]() { onVertEditPendingActionChanged(); };

		directVertEditWidget.onPendingChanged += [this]() { updateMouse(); };

		debugWidget.onRefineSingleRequested += [this](auto && ..._args) { onRefineTest(_args...); };

		canvas.depth_cull_markers = false;
		canvas.callback_mouse_left_click = [this](auto && ..._args) { return onMouseLeftClicked(_args ...); };
		canvas.callback_mouse_right_click = [this](auto && ..._args) { return onMouseRightClicked(_args ...); };
		canvas.callback_mouse_moved = [this](auto && ..._args) { return onMouseMoved(_args...); };
		canvas.callback_key_pressed = [this](auto && ..._args) { return onKeyPressed(_args...); };
		canvas.callback_key_event = [this](auto && ...) { updateMouse(); };
		canvas.callback_camera_changed = [this](auto && ..._args) { return onCameraChanged(_args...); };
		canvas.callback_custom_gui = [this](auto && ..._args) { return onDrawCustomGui(_args...); };
		canvas.callback_drop_files = [this](std::vector<std::string> _files) { onFilesDropped(_files); };

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		dagViewerWidget.onDraw += [this]() { onDagViewerDraw(); };
#endif
		requestDagViewerUpdate();

		themer.onThemeChange += [this]() { onThemeChanged(); };

		debugWidget.updateTheme();

	}

	App::~App()
	{
		for (Widget* const widget : m_widgets)
		{
			delete widget;
		}
	}

	int App::launch()
	{
		canvas.print_key_bindings();
		printUsage();
		try
		{
			return canvas.launch({}, false);
		}
		catch (...)
		{
			const std::time_t time{ std::time(nullptr) };
			const std::tm* now{ std::localtime(&time) };
			std::ostringstream filenameSs{};
			filenameSs << "crash_"
				<< std::put_time(now, "%H-%M-%S_%d-%m-%y")
				<< ".hmp";
			{
				const std::string filename{ filenameSs.str() };
				onSaveState(filename);
				std::cout << "Wrote state to " << std::filesystem::absolute(filename) << std::endl;
			}
			throw;
		}
	}

}