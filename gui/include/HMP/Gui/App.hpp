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
#include <HMP/Gui/Widgets/Axes.hpp>
#include <HMP/Gui/Widgets/Commander.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/Widgets/DirectVertEdit.hpp>
#include <HMP/Gui/Widgets/Projection.hpp>
#include <HMP/Gui/Widgets/Save.hpp>
#include <HMP/Gui/Widgets/Debug.hpp>
#include <vector>

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
#include <HMP/Gui/DagViewer/Widget.hpp>
#endif

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
#include <HMP/Gui/Widgets/Ae3d2ShapeExporter.hpp>
#endif

namespace HMP::Gui
{

	class App final: public cpputils::mixins::ReferenceClass
	{

	private:

		static constexpr cinolib::KeyBinding c_kbCancelDirectEdit{ GLFW_KEY_ESCAPE };
		static constexpr cinolib::KeyBinding c_kbDirectTranslation{ GLFW_KEY_T };
		static constexpr cinolib::KeyBinding c_kbDirectScale{ GLFW_KEY_S };
		static constexpr cinolib::KeyBinding c_kbDirectRotation{ GLFW_KEY_R };
		static constexpr cinolib::KeyBinding c_kbExtrudeFace{ GLFW_KEY_E };
		static constexpr cinolib::KeyBinding c_kbExtrudeEdge{ GLFW_KEY_E, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbExtrudeVertex{ GLFW_KEY_E, GLFW_MOD_ALT | GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbExtrudeSelected{ GLFW_KEY_E, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbRefine{ GLFW_KEY_H };
		static constexpr cinolib::KeyBinding c_kbDoubleRefine{ GLFW_KEY_H, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbFaceRefine{ GLFW_KEY_F };
		static constexpr cinolib::KeyBinding c_kbSave{ GLFW_KEY_S, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbSaveNew{ GLFW_KEY_S, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbOpen{ GLFW_KEY_O, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbLoadTarget{ GLFW_KEY_L, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbDelete{ GLFW_KEY_D };
		static constexpr cinolib::KeyBinding c_kbCopy{ GLFW_KEY_C };
		static constexpr cinolib::KeyBinding c_kbPasteFace{ GLFW_KEY_V };
		static constexpr cinolib::KeyBinding c_kbPasteEdge{ GLFW_KEY_V, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbPasteVertex{ GLFW_KEY_V, GLFW_MOD_ALT | GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbMakeConforming{ GLFW_KEY_Q };
		static constexpr cinolib::KeyBinding c_kbToggleTargetVisibility{ GLFW_KEY_U };
		static constexpr cinolib::KeyBinding c_kbUndo{ GLFW_KEY_Z, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbRedo{ GLFW_KEY_Z, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbClear{ GLFW_KEY_N, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbSelectVertex{ GLFW_KEY_1 };
		static constexpr cinolib::KeyBinding c_kbSelectEdge{ GLFW_KEY_2 };
		static constexpr cinolib::KeyBinding c_kbSelectUpEdge{ GLFW_KEY_2, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbSelectFace{ GLFW_KEY_3 };
		static constexpr cinolib::KeyBinding c_kbSelectUpFace{ GLFW_KEY_3, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbSelectPoly{ GLFW_KEY_4 };
		static constexpr cinolib::KeyBinding c_kbDeselectAll{ GLFW_KEY_A, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbSelectAll{ GLFW_KEY_A, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbAddPathEdge{ GLFW_KEY_I, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbRemovePathEdge{ GLFW_KEY_I, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbRefineTest{ GLFW_KEY_COMMA };
		static constexpr int c_kmodSelectAdd{ GLFW_MOD_SHIFT };
		static constexpr int c_kmodSelectRemove{ GLFW_MOD_CONTROL };
		static constexpr int c_kbDirectEditX{ GLFW_KEY_LEFT_CONTROL };
		static constexpr int c_kbDirectEditY{ GLFW_KEY_LEFT_SHIFT };
		static constexpr int c_kbDirectEditZ{ GLFW_KEY_LEFT_ALT };

		static void printKeyBindings();

		enum class ESelectionSource
		{
			Vertex, Edge, Face, Poly, UpFace, UpEdge
		};

		enum class ESelectionMode
		{
			Add, Remove, Set
		};

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

		HMP::Project m_project;
		cinolib::GLcanvas m_canvas;
		Meshing::Mesher& m_mesher;
		const Meshing::Mesher::Mesh& m_mesh;
		Commander& m_commander;
		cpputils::collections::SetNamer<const HMP::Dag::Node*> m_dagNamer;
		Widgets::Commander m_commanderWidget;
		Widgets::Axes m_axesWidget;
		Widgets::Target m_targetWidget;
		Widgets::VertEdit m_vertEditWidget;
		Widgets::DirectVertEdit m_directVertEditWidget;
		Widgets::Save m_saveWidget;
		Widgets::Projection m_projectionWidget;
		Widgets::Debug m_debugWidget;

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
		DagViewer::Widget m_dagViewerWidget;
		bool m_dagViewerNeedsUpdate;
#endif

#ifdef HMP_GUI_ENABLE_AE3D2SHAPE_EXPORTER
		Widgets::Ae3d2ShapeExporter m_ae3d2ShapeExporter;
#endif

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
		void onExportMesh(const std::string& _filename);

		// user operation
		void onSetPathEdge(bool _add);
		void onExtrude(Dag::Extrude::ESource _source);
		void onExtrudeSelected();
		void onCopy();
		void onPaste(Dag::Extrude::ESource _source);
		void onRefineElement(bool _twice);
		void onDelete();
		void onRefineFace();
		void onRefineTest();
		void onMakeConformant();
		void onExportMesh();
		void onSaveState();
		void onSaveNewState();
		void onLoadState();
		void onLoadTargetMesh();
		void onToggleTargetVisibility();
		void onProjectToTarget(const cinolib::Polygonmesh<>& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options);
		void onApplyTargetTransform(const Mat4& _transform);
		void onUndo();
		void onRedo();
		void onClear();
		void onSelect(ESelectionSource _source, ESelectionMode _mode);
		void onSelectAll(bool _selected);

	public:

		App();

		void loadTargetMeshOrProjectFile(const std::string& _file);

		int launch();

	};

}