#pragma once

#include <HMP/Project.hpp>
#include <HMP/Gui/Dag/Viewer.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include <cinolib/color.h>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/SetNamer.hpp>
#include <string>
#include <HMP/Gui/Widgets/Axes.hpp>
#include <HMP/Gui/Widgets/Commander.hpp>
#include <HMP/Gui/Widgets/Target.hpp>

namespace HMP::Gui
{

	class App final : public cpputils::mixins::ReferenceClass
	{

	private:

		static constexpr cinolib::Color c_backgroundColor{ cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.1f) };
		static constexpr cinolib::Color c_overlayColor{ cinolib::Color::hsv2rgb(0.1f, 0.5f, 1.0f) };
		static constexpr cinolib::Color c_mutedOverlayColor{ cinolib::Color::hsv2rgb(0.1f, 0.0f, 1.0f, 0.25f) };
		static constexpr cinolib::Color c_selectedPolyColor{ cinolib::Color::hsv2rgb(0.1f, 0.75f, 0.5f) };
		static constexpr cinolib::Color c_selectedFaceColor{ cinolib::Color::hsv2rgb(0.1f, 0.75f, 1.0f) };
		
		static constexpr std::size_t c_mouseMarkerSetInd{0};
		static constexpr std::size_t c_vertSelectionMarkerSetInd{1};
		static constexpr std::size_t c_elementsMarkerSetInd{2};

		struct
		{
			cinolib::vec2d position{};
			Vec worldPosition{};
			HMP::Dag::Element* element{};
			Id faceOffset{}, upFaceOffset{}, vertOffset{};
		} m_mouse;

		struct
		{
			HMP::Dag::Element* element{};
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
		Widgets::Commander m_commanderWidget;
		Widgets::Axes m_axesWidget;
		Widgets::Target m_targetWidget;
		bool m_dagViewerNeedsUpdate;

		void updateMouseMarkers();
		void updateVertSelectionMarkers();
		void updateElementsMarkers();
		void updateAllMarkers();

		void updateMouse();
		void updateDagViewer();

		void onCameraChange();
		bool onKeyPress(int _key, int _modifiers);
		bool onMouseMove(double _x, double _y);
		void onDrawControls();
		void onDrawCustomGui();
		void onDagViewerDraw();

		void onExtrude();
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
		void onToggleTargetVisibility();
		void onProjectToTarget();
		void onApplyTargetTransform(const Mat4& _transform);
		void onUndo();
		void onRedo();
		void onClear();

	public:

		App();

		int launch();

	};

}