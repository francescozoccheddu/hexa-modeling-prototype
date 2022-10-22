#include <HMP/Gui/Widgets/Commander.hpp>

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <cinolib/deg_rad.h>
#include <cinolib/gl/glcanvas.h>
#include <cmath>
#include <array>
#include <algorithm>

namespace HMP::Gui::Widgets
{

	Commander::Commander(HMP::Commander& _commander, HrDescriptions::DagNamer& _dagNamer)
		: m_commander{ _commander }, m_dagNamer{ _dagNamer }, cinolib::SideBarItem{ "Commander" }
	{}

	HMP::Commander& Commander::commander()
	{
		return m_commander;
	}

	const HMP::Commander& Commander::commander() const
	{
		return m_commander;
	}

	HrDescriptions::DagNamer& Commander::dagNamer()
	{
		return m_dagNamer;
	}

	const HrDescriptions::DagNamer& Commander::dagNamer() const
	{
		return m_dagNamer;
	}

	void Commander::draw()
	{
		constexpr auto actionsControl{ [](HMP::Commander::Stack& _stack, const std::string& _name) {
			int limit{ static_cast<int>(_stack.limit()) };
			ImGui::SliderInt((_name + " limit").c_str(), &limit, 0, 100, "Max %d actions", ImGuiSliderFlags_AlwaysClamp);
			_stack.limit(limit);
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

			if (m_commander.applied().empty() && m_commander.unapplied().empty())
			{
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "empty");
			}

			auto it{ m_commander.unapplied().rbegin() };
			const auto end{ m_commander.unapplied().rend() };
			while (it != end)
			{
				ImGui::TextColored(ImVec4(0.75f, 0.2f, 0.2f, 1.0f), "%s", HrDescriptions::describe(*it, m_dagNamer).c_str());
				++it;
			}

			for (const HMP::Commander::Action& action : m_commander.applied())
			{
				ImGui::TextColored(ImVec4(0.2f, 0.75f, 0.2f, 1.0f), "%s", HrDescriptions::describe(action, m_dagNamer).c_str());
			}

			ImGui::TreePop();
		}
	}

}