#pragma once

#include <HMP/Gui/Dag/Layout.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <cpputils/collections/Namer.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>

namespace HMP::Gui::Dag
{

	class Viewer final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		const Meshing::Mesher& m_mesher;
		cpputils::collections::Namer<const HMP::Dag::Node*>& m_namer;

		Layout m_layout;
		Vec2 m_center_nl{ 0.5, 0.5 };
		Real m_windowHeight_n{ 1.0 };

		void zoom(Real _amount);
		void pan(const cinolib::vec2d& _amount);
		void clampView();

	public:

		cpputils::collections::Event<Viewer> onDraw{};

		Viewer(const Meshing::Mesher& _mesher, cpputils::collections::Namer<const HMP::Dag::Node*>& _namer);

		const HMP::Dag::Element* highlight{};

		const Meshing::Mesher& mesher() const;

		Layout& layout() ;
		const Layout& layout() const;

		const cpputils::collections::Namer<const HMP::Dag::Node*>& namer() const;

		void resetView();

		void draw() override;

	};

}