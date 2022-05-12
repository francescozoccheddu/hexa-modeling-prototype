#include <HMP/Gui/Widgets.hpp>

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <cinolib/deg_rad.h>
#include <cinolib/gl/glcanvas.h>
#include <cmath>
#include <array>
#include <algorithm>

namespace HMP::Gui::Widgets
{

	void drawCommanderControls(Commander& _commander, HrDescriptions::DagNamer& _dagNamer)
	{
		constexpr auto actionsControl{ [](Commander::Stack& _stack, const std::string& _name) {
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
		actionsControl(_commander.applied(), "Undo");
		actionsControl(_commander.unapplied(), "Redo");
		ImGui::Spacing();
		// history
		if (ImGui::TreeNode("History"))
		{

			if (_commander.applied().empty() && _commander.unapplied().empty())
			{
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "empty");
			}

			auto it{ _commander.unapplied().rbegin() };
			const auto end{ _commander.unapplied().rend() };
			while (it != end)
			{
				ImGui::TextColored(ImVec4(0.75f, 0.2f, 0.2f, 1.0f), HrDescriptions::describe(*it, _dagNamer).c_str());
				++it;
			}

			for (const Commander::Action& action : _commander.applied())
			{
				ImGui::TextColored(ImVec4(0.2f, 0.75f, 0.2f, 1.0f), HrDescriptions::describe(action, _dagNamer).c_str());
			}

			ImGui::TreePop();
		}
	}

	void drawAxes(const cinolib::FreeCamera<Real>& _camera)
	{
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		Vec origin;
		Real radius;
		if (_camera.projection.perspective)
		{
			origin = _camera.view.centerAt(3);
			radius = std::tan(cinolib::to_rad(_camera.projection.verticalFieldOfView / 2));
		}
		else
		{
			origin = _camera.view.centerAt(2);
			radius = _camera.projection.verticalFieldOfView / 2 * 0.75;
		}
		const Vec right(origin + cinolib::GLcanvas::world_right * radius);
		const Vec up(origin + cinolib::GLcanvas::world_up * radius);
		const Vec forward(origin + cinolib::GLcanvas::world_forward * radius);
		const auto project{ [&_camera](const Vec& _point) -> Vec {
			const Vec4 homoProj(_camera.projectionViewMatrix() * _point.add_coord(1));
			Vec proj((homoProj / homoProj[3]).rem_coord());
			proj.x() *= _camera.projection.aspectRatio;
			const Real size{ 100 };
			const ImVec2 windowOrigin{ ImGui::GetWindowPos() };
			const ImVec2 windowSize{ ImGui::GetWindowSize() };
			proj.x() = proj.x() * size + windowOrigin.x + windowSize.x - size;
			proj.y() = -proj.y() * size + windowOrigin.y + windowSize.y - size;
			return proj;
		} };
		constexpr auto toImVec{ [](const Vec& _vec) {
			return ImVec2{static_cast<float>(_vec.x()), static_cast<float>(_vec.y())};
		} };
		origin = project(origin);
		std::array<std::pair<Vec, ImColor>, 3> tips{
			std::pair<Vec, ImColor>{project(right), IM_COL32(255,100,100,255)},
			std::pair<Vec, ImColor>{project(up), IM_COL32(100,255,100,255)},
			std::pair<Vec, ImColor>{project(forward), IM_COL32(100,100,255,255)}
		};
		std::sort(tips.begin(), tips.end(), [](const std::pair<Vec, ImColor>& _a, const std::pair<Vec, ImColor>& _b) { return _a.first.z() > _b.first.z(); });
		for (const auto& [tip, color] : tips)
		{
			drawList.AddLine(toImVec(origin), toImVec(tip), color, 3);
			drawList.AddCircleFilled(toImVec(tip), 5, color, 6);
		}
	}

}