#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <cpputils/collections/Event.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Algorithms/Projection.hpp>
#include <HMP/Gui/Widgets/Target.hpp>

namespace HMP::Gui::Widgets
{

	class Projection final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		Algorithms::Projection::Options m_options;
		const Widgets::Target& m_targetWidget;

	public:

		Projection(const Widgets::Target& _targetWidget);

		cpputils::collections::Event<Projection, const Algorithms::Projection::Options&> onProjectRequest;

		const Algorithms::Projection::Options& options() const;

		void requestProjection();

		void draw() override;

	};

}