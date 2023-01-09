#include <HMP/Gui/SidebarWidget.hpp>

namespace HMP::Gui
{

	void SidebarWidget::draw()
	{
		drawSidebar();
	}

	SidebarWidget::SidebarWidget(const std::string& _title) : cinolib::SideBarItem{ _title }
	{}

}