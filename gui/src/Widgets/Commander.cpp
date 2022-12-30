#include <HMP/Gui/Widgets/Commander.hpp>

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <imgui.h>
#include <cinolib/deg_rad.h>
#include <cinolib/gl/glcanvas.h>
#include <cmath>
#include <array>
#include <algorithm>
#include <HMP/Gui/themer.hpp>

namespace HMP::Gui::Widgets
{

	Commander::Commander(HMP::Commander& _commander, Utils::HrDescriptions::DagNamer& _dagNamer, const VertEdit& _vertEdit)
		: cinolib::SideBarItem{ "Commander" }, m_commander{ _commander }, m_dagNamer{ _dagNamer }, m_vertEdit{ _vertEdit }
	{}

	const VertEdit& Commander::vertEdit() const
	{
		return m_vertEdit;
	}

	HMP::Commander& Commander::commander()
	{
		return m_commander;
	}

	const HMP::Commander& Commander::commander() const
	{
		return m_commander;
	}

	Utils::HrDescriptions::DagNamer& Commander::dagNamer()
	{
		return m_dagNamer;
	}

	const Utils::HrDescriptions::DagNamer& Commander::dagNamer() const
	{
		return m_dagNamer;
	}

	void Commander::draw()
	{
		constexpr auto actionsControl{ [](HMP::Commander::Stack& _stack, const std::string& _name) {
			int limit{ static_cast<int>(_stack.limit()) };
			ImGui::SliderInt((_name + " limit").c_str(), &limit, 0, 100, "Max %d actions", ImGuiSliderFlags_AlwaysClamp);
			_stack.limit(static_cast<I>(limit));
			if (!_stack.empty())
			{
				ImGui::SameLine();
				if (ImGui::Button((std::string{ "Clear " } + std::to_string(_stack.size()) + " actions").c_str()))
				{
					_stack.clear();
				}
			}
		} };
		actionsControl(m_commander.applied(), "Undo");
		actionsControl(m_commander.unapplied(), "Redo");
		ImGui::Spacing();
		// history
		if (ImGui::TreeNode("History"))
		{

			if (m_commander.applied().empty() && m_commander.unapplied().empty() && !m_vertEdit.pendingAction())
			{
				ImGui::TextDisabled("empty");
			}

			if (m_vertEdit.pendingAction())
			{
				ImGui::TextColored(Utils::Controls::toImVec4(themer->commanderPendingActionColor), "Pending vertex edit action on %d vertices", static_cast<int>(m_vertEdit.vids().size()));
			}

			for (const auto& action : m_commander.unapplied().reverse())
			{
				ImGui::TextColored(Utils::Controls::toImVec4(themer->commanderUnappliedActionColor), "%s", Utils::HrDescriptions::describe(action, m_dagNamer).c_str());
			}

			for (const HMP::Commander::Action& action : m_commander.applied())
			{
				ImGui::TextColored(Utils::Controls::toImVec4(themer->commanderAppliedActionColor), "%s", Utils::HrDescriptions::describe(action, m_dagNamer).c_str());
			}

			ImGui::TreePop();
		}
	}

}