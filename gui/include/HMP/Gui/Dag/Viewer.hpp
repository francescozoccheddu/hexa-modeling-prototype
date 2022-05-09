#pragma once

#include <HMP/Gui/Dag/Layout.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/side_bar_item.h>

namespace HMP::Gui::Dag
{

	class Viewer final : public cinolib::SideBarItem
	{

	private:

		const Meshing::Mesher& m_mesher;

		Vec2 m_center_nl{ 0.5, 0.5 };
		Real m_windowHeight_n{ 1.0 };

		void zoom(Real _amount);
		void pan(const cinolib::vec2d& _amount);
		void clampView();

	public:

		Viewer(const Meshing::Mesher& _mesher);

		const HMP::Dag::Element* highlight{};

		const Meshing::Mesher& mesher() const;

		Layout layout{};

		void resetView();

		void draw() override;

	};

}