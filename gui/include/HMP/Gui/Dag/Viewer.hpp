#pragma once

#include <HMP/Gui/Dag/Layout.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Gui::Dag
{

	class Viewer final
	{

	private:

		const Meshing::Mesher& m_mesher;

		cinolib::vec2d m_center_nl{ 0.5, 0.5 };
		double m_windowHeight_n{ 1.0 };

		void zoom(double _amount);
		void pan(const cinolib::vec2d& _amount);
		void clampView();

	public:

		Viewer(const Meshing::Mesher& _mesher);

		const HMP::Dag::Element* highlight{};

		const Meshing::Mesher& mesher() const;

		Layout layout{};

		void resetView();

		void draw();

	};

}