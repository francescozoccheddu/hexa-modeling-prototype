#pragma once

#include <HMP/Project.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/gl_glfw.h>
#include <cinolib/gl/key_bindings.hpp>
#include <cinolib/color.h>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/SetNamer.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <string>
#include <vector>
#include <HMP/Gui/Widget.hpp>
#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Projection/project.hpp>
#include <HMP/Refinement/Schemes.hpp>

namespace HMP::Gui
{

	namespace DagViewer
	{
		class Widget;
	}

	namespace Widgets
	{

		class Axes;
		class Debug;
		class Save;
		class Pad;
		class VertEdit;
		class DirectVertEdit;
		class Target;
		class Projection;
		class Smooth;
		class Ae3d2ShapeExporter;
		class Commander;
		class Actions;
		class Highlight;

	}

	class App final : public cpputils::mixins::ReferenceClass
	{

	public:

		struct Mouse final
		{
			cinolib::vec2d position{};
			Dag::Element* element{};
			I fi{}, vi{}, ei{};
			Id pid{ noId }, fid{ noId }, vid{ noId }, eid{ noId };
		};

	private:

		void printUsage() const;


	public:

		HMP::Project project;
		cinolib::GLcanvas canvas;
		Meshing::Mesher& mesher;
		Commander& commander;
		cpputils::collections::SetNamer<const HMP::Dag::Node*> dagNamer;

		Widgets::Actions& actionsWidget;
		Widgets::Commander& commanderWidget;
		Widgets::Axes& axesWidget;
		Widgets::Target& targetWidget;
		Widgets::VertEdit& vertEditWidget;
		Widgets::DirectVertEdit& directVertEditWidget;
		Widgets::Save& saveWidget;
		Widgets::Projection& projectionWidget;
		Widgets::Debug& debugWidget;
		Widgets::Pad& padWidget;
		Widgets::Smooth& smoothWidget;
		Widgets::Highlight& highlightWidget;

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		DagViewer::Widget& dagViewerWidget;
#endif

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
		Widgets::Ae3d2ShapeExporter& ae3d2ShapeExporter;
#endif

	private:

		const std::vector<Widget*> m_widgets;
		Mouse m_mouse{};

		void resetMouse();

		// actions
		void onActionApplied();

		// other events
		void onThemeChanged();

		// mesher events
		void onMesherRestored(const Meshing::Mesher::State& _state);
		void onMesherElementVisibilityChanged(const Dag::Element& _element, bool _visible);

		// vert edit events
		void onApplyVertEdit(const std::vector<Id>& _vids, const Mat4& _transform);
		void onVertEditPendingActionChanged();

		// canvas events
		void onCameraChanged();
		bool onMouseLeftClicked(int _modifiers);
		bool onMouseRightClicked(int _modifiers);
		bool onKeyPressed(int _key, int _modifiers);
		bool onMouseMoved(double _x, double _y);
		void updateMouse();
		void onFilesDropped(const std::vector<std::string>& _files);

		// save events
		void onSaveState(const std::string& _filename);
		void onLoadState(const std::string& _filename);
		void onExportMesh(const std::string& _filename) const;

		// user operation
		void onProjectToTarget(const cinolib::Polygonmesh<>& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options);
		void onApplyTargetTransform(const Mat4& _transform);
		void onRefineTest(Refinement::EScheme _scheme, I _forwardFi, I _firstVi);
		void onPad(Real _length, I _smoothIterations, Real _smoothSurfVertWeight, Real _cornerShrinkFactor);
		void onSmooth(I _surfaceIterations, I _internalIterations, Real _surfVertWeight);

	public:

		Dag::Element* copiedElement{};

		App();

		~App();

		void redo();
		void applyAction(Commander::Action& _action);
		void undo();

		void loadTargetMeshOrProjectFile(const std::string& _file);

		void serialize(const std::string& _filename);

		void deserialize(const std::string& _filename);

		const Mouse& mouse() const;

		int launch();

	};

}