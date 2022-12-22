#pragma once

#include <HMP/Gui/DagViewer/Layout.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <cpputils/collections/Namer.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>

namespace HMP::Gui::DagViewer
{

	class Widget final: public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		static void initFonts();

		const Meshing::Mesher& m_mesher;
		cpputils::collections::Namer<const Dag::Node*>& m_namer;

		Layout m_layout;
		Vec2 m_center_nl{ 0.5, 0.5 };
		Real m_windowHeight_n{ 1.0 };

		void zoom(Real _amount);
		void pan(const cinolib::vec2d& _amount);
		void clampView();

	public:

		cpputils::collections::Event<Widget> onDraw{};

		Widget(const Meshing::Mesher& _mesher, cpputils::collections::Namer<const Dag::Node*>& _namer);

		const Dag::Element* highlight{}, * copied{};

		const Meshing::Mesher& mesher() const;

		Layout& layout();
		const Layout& layout() const;

		const cpputils::collections::Namer<const Dag::Node*>& namer() const;

		void resetView();

		void draw() override;

	};

}