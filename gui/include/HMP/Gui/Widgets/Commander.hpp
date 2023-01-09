#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>
#include <HMP/Gui/Utils/HrDescriptions.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/SidebarWidget.hpp>
#include <cinolib/color.h>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP::Gui::Widgets
{

	class Commander final: public SidebarWidget
	{

	private:

		HMP::Commander& m_commander;
		Utils::HrDescriptions::DagNamer& m_dagNamer;
		const VertEdit& m_vertEdit;

		void drawSidebar() override;

	public:

		Commander(HMP::Commander& _commander, Utils::HrDescriptions::DagNamer& _dagNamer, const VertEdit& _vertEdit);

	};

}