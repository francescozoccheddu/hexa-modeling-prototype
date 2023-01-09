#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Widget.hpp>
#include <cinolib/gl/FreeCamera.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP::Gui::Widgets
{

	class Axes final: public Widget
	{

	private:

		void drawCanvas() override;

	};

}