#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Gui/DagViewer/Layout.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Meshing/types.hpp>
#include <cpputils/collections/Namer.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <cinolib/color.h>

namespace HMP::Gui::DagViewer
{

	class Widget final: public SidebarWidget
	{

	private:

		static void initFonts();

		cpputils::collections::Namer<const Dag::Node*>& m_namer;

		Vec2 m_center_nl{ 0.5, 0.5 };
		Real m_windowHeight_n{ 1.0 };
		bool m_showLayoutPerformanceWarning{ false };

		void zoom(Real _amount);
		void pan(const cinolib::vec2d& _amount);
		void clampView();

		void drawTooltip(const Dag::Node& _node) const;

		const Dag::Node* m_hovered{};

		void drawSidebar() override;

		void draw(const cinolib::GLcanvas& _canvas) override;

	public:


		Layout layout;
		const Dag::Element* highlight{}, * copied{};
		bool tooManyNodes{ false };

		mutable cpputils::collections::Event<Widget> onDraw{};

		Widget(cpputils::collections::Namer<const Dag::Node*>& _namer);

		void resetView();

		bool hasHoveredNode() const;

		const Dag::Node& hoveredNode() const;

	};

}