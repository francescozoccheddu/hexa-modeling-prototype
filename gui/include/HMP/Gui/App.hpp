#pragma once

#include <HMP/Project.hpp>
#include <HMP/Gui/Dag/Viewer.hpp>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/meshes/drawable_trimesh.h>
#include <cinolib/color.h>
#include <string>

namespace HMP::Gui
{

	class App final
	{

	private:

		inline static const unsigned int c_highlightVertexRadius{ 10u };

		struct
		{
			cinolib::vec2d position{};
		} m_mouse;

		struct
		{
			Id vid{};
			bool pending{ false };
		} m_move;

		struct
		{
			Id pid{};
			bool pending{ false };
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

		void updateHighlight();
		void updateDagViewer();

		void onCameraChange();
		bool onKeyPress(int _key, int _modifiers);
		bool onMouseMove(double _x, double _y);
		void onDrawControls();

		void onMove();
		void onExtrude();
		void onCopy();
		void onPaste();
		void onRefineHexahedron();
		void onDeleteHexahedron();
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