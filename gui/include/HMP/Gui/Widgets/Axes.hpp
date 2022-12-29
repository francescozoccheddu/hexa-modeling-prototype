#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/FreeCamera.hpp>
#include <cinolib/gl/canvas_gui_item.h>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP::Gui::Widgets
{

	class Axes final: public cinolib::CanvasGuiItem, public cpputils::mixins::ReferenceClass
	{

	public:

		float colorSat{ 1.0f }, colorVal{ 1.0f };

		void draw(const cinolib::GLcanvas& _canvas) override;

	};

}