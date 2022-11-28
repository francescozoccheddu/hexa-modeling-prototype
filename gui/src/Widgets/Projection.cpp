#include <HMP/Gui/Widgets/Projection.hpp>

#include <imgui.h>
#include <utility>
#include <stdexcept>
#include <HMP/Actions/Project.hpp>

namespace HMP::Gui::Widgets
{

	Projection::Projection(const Widgets::Target& _targetWidget, HMP::Commander& _commander) :
		cinolib::SideBarItem{ "Projection" }, m_targetWidget{ _targetWidget }, m_commander{ _commander },
		onProjectRequest{}, m_options{}
	{}

	const Algorithms::Projection::Options& Projection::options() const
	{
		return m_options;
	}

	void Projection::requestProjection()
	{
		if (!m_targetWidget.hasMesh())
		{
			throw std::logic_error{ "no target mesh" };
		}
		onProjectRequest(m_options);
	}

	bool Projection::canReproject() const
	{
		if (!m_commander.canUndo())
		{
			return false;
		}
		return dynamic_cast<const Actions::Project*>(&m_commander.applied().first());
	}

	void Projection::requestReprojection()
	{
		if (!canReproject())
		{
			throw std::logic_error{ "no projection to undo" };
		}
		m_commander.undo();
		requestProjection();
	}

	void Projection::draw()
	{
		static constexpr auto tweak{ [](Algorithms::Projection::Tweak& _tweak, const char* _header) {
			ImGui::TextDisabled("%s", _header);
			float min{static_cast<float>(_tweak.min())};
			ImGui::SliderFloat("Min", &min, -2.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			float power{static_cast<float>(_tweak.power())};
			ImGui::SliderFloat("Power", &power, 0.0f, 4.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
			_tweak = { min, power };
		} };
		static constexpr auto space{ []() {
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
		} };
		{
			int iterations{ static_cast<int>(m_options.iterations) };
			if (ImGui::SliderInt("Iterations", &iterations, 1, 10, "%d", ImGuiSliderFlags_AlwaysClamp))
			{
				m_options.iterations = static_cast<std::size_t>(iterations);
			}
		}
		space();
		{
			int invertMode{ static_cast<int>(m_options.invertMode) };
			if (ImGui::Combo("Invert mode", &invertMode, "Distance\0BarycentricCoords\0"))
			{
				m_options.invertMode = static_cast<Algorithms::Projection::EInvertMode>(invertMode);
			}
			int displaceMode{ static_cast<int>(m_options.displaceMode) };
			if (ImGui::Combo("Displace mode", &displaceMode, "NormDirAvgAndDirNormAvg\0NormDirAvgAndDirAvg\0DirAvg\0VertAvg\0"))
			{
				m_options.displaceMode = static_cast<Algorithms::Projection::EDisplaceMode>(displaceMode);
			}
		}
		space();
		tweak(m_options.weightTweak, "Weight factor");
		space();
		tweak(m_options.normalDotTweak, "Normal dot factor");
		space();
		tweak(m_options.unsetVertsDistWeightTweak, "Unset verts distance factor");
		space();
		{
			float advancePercentile{ static_cast<float>(m_options.advancePercentile) };
			if (ImGui::SliderFloat("Advance percentile", &advancePercentile, 0.0f, 100.0f, "%.2f%%", ImGuiSliderFlags_AlwaysClamp))
			{
				m_options.advancePercentile = advancePercentile;
			}
		}
		space();
		ImGui::Checkbox("Smooth", &m_options.smooth);
		if (m_targetWidget.hasMesh())
		{
			space();
			if (ImGui::Button("Project"))
			{
				requestProjection();
			}
			if (canReproject())
			{
				ImGui::SameLine();
				if (ImGui::Button("Reproject"))
				{
					requestReprojection();
				}
			}
		}
	}

}
