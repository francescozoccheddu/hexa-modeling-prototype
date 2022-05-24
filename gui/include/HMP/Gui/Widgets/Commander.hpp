#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>
#include <HMP/Gui/HrDescriptions.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP::Gui::Widgets
{

	class Commander final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		HMP::Commander& m_commander;
		HrDescriptions::DagNamer& m_dagNamer;

	public:
		
		Commander(HMP::Commander& _commander, HrDescriptions::DagNamer& _dagNamer);

		HMP::Commander& commander();
		const HMP::Commander& commander() const;

		HrDescriptions::DagNamer& dagNamer();
		const HrDescriptions::DagNamer& dagNamer() const;

		void draw() override;

	};

}