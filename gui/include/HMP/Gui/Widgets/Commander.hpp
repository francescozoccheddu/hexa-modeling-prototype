#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>
#include <HMP/Gui/HrDescriptions.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP::Gui::Widgets
{

	class Commander final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		HMP::Commander& m_commander;
		HrDescriptions::DagNamer& m_dagNamer;
		const VertEdit& m_vertEdit;

	public:
		
		Commander(HMP::Commander& _commander, HrDescriptions::DagNamer& _dagNamer, const VertEdit& _vertEdit);

		const VertEdit& vertEdit() const;

		HMP::Commander& commander();
		const HMP::Commander& commander() const;

		HrDescriptions::DagNamer& dagNamer();
		const HrDescriptions::DagNamer& dagNamer() const;

		void draw() override;

	};

}