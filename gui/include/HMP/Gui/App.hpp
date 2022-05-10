#pragma once

#include <HMP/Project.hpp>
#include <HMP/Gui/Dag/Viewer.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/gl/volume_mesh_controls.h>
#include <cinolib/meshes/drawable_trimesh.h>
#include <cinolib/color.h>
#include <string>

namespace HMP::Gui
{

	class App final
	{

	private:

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
			Id vertOffset{};
		} m_move;

		struct
		{
			HMP::Dag::Element* element{};
		} m_copy;

		struct
		{
			cinolib::DrawableTrimesh<>* mesh{ nullptr };
			std::string filename{};
		} m_target;

		HMP::Project m_project;
		cinolib::GLcanvas m_canvas;
		Meshing::Mesher& m_mesher;
		const Meshing::Mesher::Mesh& m_mesh;
		Commander& m_commander;
		Dag::Viewer m_dagViewer; 
		cinolib::VolumeMeshControls<Meshing::Mesher::Mesh> m_menu;

		void updateMouse();
		void updateDagViewer();

		void onCameraChange();
		bool onKeyPress(int _key, int _modifiers);
		bool onMouseMove(double _x, double _y);
		void onDrawControls();

		void onMove();
		void onExtrude();
		void onCopy();
		void onPaste();
		void onRefineElement();
		void onDelete();
		void onRefineFace();
		void onMakeConformant();
		void onSaveMesh();
		void onSaveTree();
		void onLoadTree();
		void onToggleTargetVisibility();
		void onProjectToTarget();
		void onUndo();
		void onRedo();
		void onClear();

	public:

		App();

		int launch();

	};

}