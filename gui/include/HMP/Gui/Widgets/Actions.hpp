#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Widget.hpp>
#include <cinolib/gl/FreeCamera.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Dag/Extrude.hpp>

namespace HMP::Gui::Widgets
{

	class Actions final: public Widget
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
		static constexpr cinolib::KeyBinding c_kbClear{ GLFW_KEY_N, GLFW_MOD_CONTROL };
		static constexpr cinolib::KeyBinding c_kbSubdivideAll{ GLFW_KEY_0, GLFW_MOD_CONTROL };

		bool keyPressed(const cinolib::KeyBinding& _key) override;

		void printUsage() const override;

		bool hoveredExtrudeElements(Dag::Extrude::ESource _source, cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, cpputils::collections::FixedVector<I, 3>& _fis, I& _firstVi, bool& _clockwise);
		void onExtrude(Dag::Extrude::ESource _source);
		void onExtrudeSelected();
		void onCopy();
		void onPaste(Dag::Extrude::ESource _source);
		void onRefineElement(bool _twice);
		void onDelete();
		void onRefineFace();
		void onMakeConformant();
		void onClear();
		void onSubdivideAll();

	public:

		void clear();

	};

}