#pragma once

#include <HMP/gui/dag/Layout.hpp>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Gui::Dag
{

	class Viewer final
	{

	private:

		cinolib::vec2d m_center_nl{ 0.5, 0.5 };
		double m_windowHeight_n{ 1.0 };

		void zoom(double _amount);
		void pan(const cinolib::vec2d& _amount);
		void clampView();

	public:

		int highlightedElementId{};
		bool highlight{ false };

		Layout layout{};

		void resetView();

		void draw();

	};

}