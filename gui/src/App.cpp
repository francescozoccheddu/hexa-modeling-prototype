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
#include <HMP/Gui/Widgets/Highlight.hpp>
#include <HMP/Gui/Widgets/Actions.hpp>

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
#include <HMP/Gui/DagViewer/Widget.hpp>
#include <HMP/Gui/DagViewer/createLayout.hpp>
#endif

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
#include <HMP/Gui/Widgets/Ae3d2ShapeExporter.hpp>
#endif

namespace HMP::Gui
{

	void App::printUsage() const
	{
		std::cout << "------ App key bindings -------\n";
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
		m_mouse = {};
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
			m_mouse = {};
		}
		if (copiedElement && copiedElement->pid >= mesher.mesh().num_polys())
		{
			copiedElement = nullptr;
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
		return false;
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

	void App::onDagViewerDraw()
	{
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		if (m_dagViewerNeedsUpdate)
		{
			m_dagViewerNeedsUpdate = false;
			if (project.root() && mesher.mesh().num_polys() < 100000)
			{
				dagViewerWidget.tooManyNodes = false;
				dagViewerWidget.layout = DagViewer::createLayout(*project.root());
			}
			else
			{
				dagViewerWidget.tooManyNodes = true;
			}
			dagViewerWidget.resetView();
		}
#endif
	}

	const App::Mouse& App::mouse() const
	{
		return m_mouse;
	}

	void App::updateMouse()
	{
		m_mouse = {};
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

	void App::onRefineTest(Refinement::EScheme _scheme, I _forwardFi, I _firstVi)
	{
		if (mesher.mesh().num_polys() == 1)
		{
			applyAction(*new Actions::Refine{ mesher.element(0), _forwardFi, _firstVi, _scheme });
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

	void App::undo()
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

	void App::redo()
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
		commanderWidget{ *new Widgets::Commander{} },
		axesWidget{ *new Widgets::Axes{} },
		targetWidget{ *new Widgets::Target{mesher.mesh() } },
		vertEditWidget{ *new Widgets::VertEdit{mesher } },
		directVertEditWidget{ *new Widgets::DirectVertEdit{vertEditWidget, canvas } },
		saveWidget{ *new Widgets::Save{} },
		projectionWidget{ *new Widgets::Projection{ targetWidget, commander, mesher } },
		debugWidget{ *new Widgets::Debug{mesher, dagNamer, vertEditWidget, targetWidget } },
		padWidget{ *new Widgets::Pad{ mesher.mesh() } },
		smoothWidget{ *new Widgets::Smooth{} },
		highlightWidget{ *new Widgets::Highlight{} },
		actionsWidget{ *new Widgets::Actions{} },
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
			&highlightWidget,
			&vertEditWidget, 
			&directVertEditWidget, 
			&padWidget, 
			&smoothWidget,
			&targetWidget, 
			&projectionWidget, 
			&axesWidget, 
			&actionsWidget, 
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
		canvas.callback_drop_files = [this](std::vector<std::string> _files) { onFilesDropped(_files); };

		themer.onThemeChange += [this]() { onThemeChanged(); };

		debugWidget.updateTheme();

		actionsWidget.clear();
		commander.applied().clear();
		canvas.push(&mesher.mesh());

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		dagViewerWidget.onDraw += [this]() { onDagViewerDraw(); };
#endif
		requestDagViewerUpdate();

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