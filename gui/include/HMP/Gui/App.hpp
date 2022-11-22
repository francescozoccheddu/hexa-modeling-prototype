#pragma once

#include <HMP/Project.hpp>
#include <HMP/Gui/Dag/Viewer.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/gl_glfw.h>
#include <cinolib/gl/key_bindings.hpp>
#include <cinolib/gl/volume_mesh_controls.h>
#include <cinolib/color.h>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/SetNamer.hpp>
#include <string>
#include <HMP/Gui/Widgets/Axes.hpp>
#include <HMP/Gui/Widgets/Commander.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/Widgets/DirectVertEdit.hpp>
#include <HMP/Gui/Widgets/Ae3d2ShapeExporter.hpp>

namespace HMP::Gui
{

	class App final : public cpputils::mixins::ReferenceClass
	{

	private:

		static constexpr cinolib::Color c_warningTextColor{ cinolib::Color::hsv2rgb(0.2f, 0.6f, 0.6f) };
		static constexpr cinolib::Color c_backgroundColor{ cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.1f) };
		static constexpr cinolib::Color c_overlayColor{ cinolib::Color::hsv2rgb(0.1f, 0.5f, 1.0f) };
		static constexpr cinolib::Color c_mutedOverlayColor{ cinolib::Color::hsv2rgb(0.1f, 0.0f, 1.0f, 0.25f) };
		static constexpr cinolib::Color c_selectedPolyColor{ cinolib::Color::hsv2rgb(0.1f, 0.75f, 0.5f) };
		static constexpr cinolib::Color c_selectedFaceColor{ cinolib::Color::hsv2rgb(0.1f, 0.75f, 1.0f) };

		static constexpr std::size_t c_mouseMarkerSetInd{ 1 };
		static constexpr std::size_t c_vertSelectionMarkerSetInd{ 2 };
		static constexpr std::size_t c_elementsMarkerSetInd{ 3 };
		static constexpr std::size_t c_markerSetCount{ 4 };

		static constexpr cinolib::KeyBinding c_kbCancelDirectEdit{ GLFW_KEY_ESCAPE };
		static constexpr cinolib::KeyBinding c_kbDirectTranslation{ GLFW_KEY_T };
		static constexpr cinolib::KeyBinding c_kbDirectScale{ GLFW_KEY_S };
		static constexpr cinolib::KeyBinding c_kbDirectRotation{ GLFW_KEY_R };
		static constexpr cinolib::KeyBinding c_kbExtrude{ GLFW_KEY_E };
		static constexpr cinolib::KeyBinding c_kbExtrudeAndSelect{ GLFW_KEY_E, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbRefine{ GLFW_KEY_H };
		static constexpr cinolib::KeyBinding c_kbDoubleRefine{ GLFW_KEY_H, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbFaceRefine{ GLFW_KEY_F };
		static constexpr cinolib::KeyBinding c_kbSave{ GLFW_KEY_S, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbSaveMesh{ GLFW_KEY_S, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbOpen{ GLFW_KEY_O, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbLoadTarget{ GLFW_KEY_L, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbDelete{ GLFW_KEY_D };
		static constexpr cinolib::KeyBinding c_kbCopy{ GLFW_KEY_C };
		static constexpr cinolib::KeyBinding c_kbPaste{ GLFW_KEY_V };
		static constexpr cinolib::KeyBinding c_kbRotate{ GLFW_KEY_Y };
		static constexpr cinolib::KeyBinding c_kbMakeConforming{ GLFW_KEY_Q };
		static constexpr cinolib::KeyBinding c_kbToggleTargetVisibility{ GLFW_KEY_U };
		static constexpr cinolib::KeyBinding c_kbUndo{ GLFW_KEY_Z, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbRedo{ GLFW_KEY_Z, GLFW_MOD_CONTROL | GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbClear{ GLFW_KEY_N, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbPrintDebugInfo{ GLFW_KEY_COMMA };
		static constexpr cinolib::KeyBinding c_kbSelectVertex{ GLFW_KEY_1 };
		static constexpr cinolib::KeyBinding c_kbSelectEdge{ GLFW_KEY_2 };
		static constexpr cinolib::KeyBinding c_kbSelectUpEdge{ GLFW_KEY_2, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbSelectFace{ GLFW_KEY_3 };
		static constexpr cinolib::KeyBinding c_kbSelectUpFace{ GLFW_KEY_3, GLFW_MOD_ALT };
		static constexpr cinolib::KeyBinding c_kbSelectPoly{ GLFW_KEY_4 };
		static constexpr cinolib::KeyBinding c_kbDeselectAll{ GLFW_KEY_A, GLFW_MOD_CONTROL };
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
			Id faceOffset{}, upFaceOffset{}, vertOffset{};
		} m_mouse;

		struct
		{
			HMP::Dag::NodeHandle<HMP::Dag::Element> element{};
		} m_copy;

		struct
		{
			bool showNames{ false };
		} m_options;

		HMP::Project m_project;
		cinolib::GLcanvas m_canvas;
		Meshing::Mesher& m_mesher;
		const Meshing::Mesher::Mesh& m_mesh;
		Commander& m_commander;
		cpputils::collections::SetNamer<const HMP::Dag::Node*> m_dagNamer;
		Dag::Viewer m_dagViewer;
		cinolib::VolumeMeshControls<Meshing::Mesher::Mesh> m_menu;
		Widgets::Axes m_axesWidget;
		Widgets::Target m_targetWidget;
		Widgets::VertEdit m_vertEditWidget;
		Widgets::Commander m_commanderWidget;
		Widgets::DirectVertEdit m_directVertEditWidget;
		Widgets::Ae3d2ShapeExporter m_ae3d2ShapeExporter;
		bool m_dagViewerNeedsUpdate;

		// markers
		void updateMouseMarkers();
		void updateVertSelectionMarkers();
		void updateElementsMarkers();
		void updateAllMarkers();

		// actions
		void onActionApplied();
		void applyAction(Commander::Action& _action);
		void requestDagViewerUpdate();

		// mesher events
		void onElementRemove(const HMP::Dag::Element& _element);
		void onClearElements();

		// vert edit events
		void onVertEditVidsOrCentroidChanged();
		void onVertEditMeshUpdated();
		void onApplyVertEdit(const std::vector<Id>& _vids, const Mat4& _transform);
		void onVertEditPendingActionChanged();
		void onTargetVertInterpolationChanged(const std::unordered_map<Id, Vec>& _moves);

		// canvas events
		void onCameraChanged();
		bool onMouseLeftClicked(int _modifiers);
		bool onMouseRightClicked(int _modifiers);
		bool onKeyPressed(int _key, int _modifiers);
		bool onMouseMoved(double _x, double _y);
		void onDrawControls();
		void onDrawCustomGui();
		void onDagViewerDraw();
		void updateMouse();

		// user operation
		std::string getDebugInfo() const;
		void onPrintDebugInfo() const;
		void onExtrude();
		void onExtrudeAndSelect();
		void onCopy();
		void onPaste();
		void onRefineElement(bool _twice);
		void onDelete();
		void onRotate();
		void onRefineFace();
		void onMakeConformant();
		void onSaveMesh();
		void onSaveTree();
		void onLoadTree();
		void onLoadTargetMesh();
		void onToggleTargetVisibility();
		void onProjectToTarget();
		void onApplyTargetTransform(const Mat4& _transform);
		void onUndo();
		void onRedo();
		void onClear();
		void onSelect(ESelectionSource _source, ESelectionMode _mode);
		void onClearSelection();

	public:

		App();

		int launch();

	};

}