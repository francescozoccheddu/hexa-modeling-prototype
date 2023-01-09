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

	}

	class App final: public cpputils::mixins::ReferenceClass
	{

	private:

		static constexpr cinolib::KeyBinding c_kbExtrudeFace{ GLFW_KEY_E };
		static constexpr cinolib::KeyBinding c_kbExtrudeEdge{ GLFW_KEY_E, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbExtrudeVertex{ GLFW_KEY_E, GLFW_MOD_ALT | GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbExtrudeSelected{ GLFW_KEY_E, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbRefine{ GLFW_KEY_H };
		static constexpr cinolib::KeyBinding c_kbDoubleRefine{ GLFW_KEY_H, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbFaceRefine{ GLFW_KEY_F };
		static constexpr cinolib::KeyBinding c_kbDelete{ GLFW_KEY_D };
		static constexpr cinolib::KeyBinding c_kbCopy{ GLFW_KEY_C };
		static constexpr cinolib::KeyBinding c_kbPasteFace{ GLFW_KEY_V };
		static constexpr cinolib::KeyBinding c_kbPasteEdge{ GLFW_KEY_V, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbPasteVertex{ GLFW_KEY_V, GLFW_MOD_ALT | GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbMakeConforming{ GLFW_KEY_Q };
		static constexpr cinolib::KeyBinding c_kbUndo{ GLFW_KEY_Z, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbRedo{ GLFW_KEY_Z, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbClear{ GLFW_KEY_N, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbSubdivideAll{ GLFW_KEY_0, GLFW_MOD_CONTROL };

		void printUsage() const;

	public:

		struct
		{
			cinolib::vec2d position{};
			HMP::Dag::Element* element{};
			I fi, vi, ei;
			Id pid, fid, vid, eid;
		} m_mouse;

		struct
		{
			HMP::Dag::Element* element{};
		} m_copy;

		HMP::Project project;
		cinolib::GLcanvas canvas;
		Meshing::Mesher& mesher;
		Commander& commander;
		cpputils::collections::SetNamer<const HMP::Dag::Node*> dagNamer;

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


#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		DagViewer::Widget& dagViewerWidget;
	private:
		bool m_dagViewerNeedsUpdate;
	public:
#endif

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
		Widgets::Ae3d2ShapeExporter& ae3d2ShapeExporter;
#endif

	private:

		const std::vector<Widget*> m_widgets;

		// actions
		void onActionApplied();
		void applyAction(Commander::Action& _action);
		void requestDagViewerUpdate();

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
		void onDrawCustomGui();
		void onDagViewerDraw();
		void updateMouse();
		void onFilesDropped(const std::vector<std::string>& _files);
		bool hoveredExtrudeElements(Dag::Extrude::ESource _source, cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, cpputils::collections::FixedVector<I, 3>& _fis, I& _firstVi, bool& _clockwise);

		// save events
		void onSaveState(const std::string& _filename);
		void onLoadState(const std::string& _filename);
		void onExportMesh(const std::string& _filename) const;

		// user operation
		void onExtrude(Dag::Extrude::ESource _source);
		void onExtrudeSelected();
		void onCopy();
		void onPaste(Dag::Extrude::ESource _source);
		void onRefineElement(bool _twice);
		void onDelete();
		void onRefineFace();
		void onRefineTest(Refinement::EScheme _scheme, I _forwardFi, I _firstVi);
		void onMakeConformant();
		void onToggleTargetVisibility();
		void onProjectToTarget(const cinolib::Polygonmesh<>& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options);
		void onApplyTargetTransform(const Mat4& _transform);
		void onUndo();
		void onRedo();
		void onClear();
		void onSubdivideAll();
		void onPad(Real _length, I _smoothIterations, Real _smoothSurfVertWeight, Real _cornerShrinkFactor);
		void onSmooth(I _surfaceIterations, I _internalIterations, Real _surfVertWeight);

	public:

		App();

		~App();

		void loadTargetMeshOrProjectFile(const std::string& _file);

		void serialize(const std::string& _filename);

		void deserialize(const std::string& _filename);

		int launch();

	};

}