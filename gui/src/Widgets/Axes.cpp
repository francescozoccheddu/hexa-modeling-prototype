#include <HMP/Gui/Widgets/Axes.hpp>

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <cinolib/deg_rad.h>
#include <cinolib/gl/glcanvas.h>
#include <cmath>
#include <array>
#include <utility>
#include <algorithm>

namespace HMP::Gui::Widgets
{

	Axes::Axes(const cinolib::FreeCamera<Real>& _camera)
		: m_camera{ _camera }
	{}

	const cinolib::FreeCamera<Real>& Axes::camera() const
	{
		return m_camera;
	}

	void Axes::draw()
	{
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		Vec origin;
		Real radius;
		if (m_camera.projection.perspective)
		{
			origin = m_camera.view.centerAt(3);
			radius = std::tan(cinolib::to_rad(m_camera.projection.verticalFieldOfView / 2));
		}
		else
		{
			origin = m_camera.view.centerAt(2);
			radius = m_camera.projection.verticalFieldOfView / 2 * 0.75;
		}
		const Vec right(origin + cinolib::GLcanvas::world_right() * radius);
		const Vec up(origin + cinolib::GLcanvas::world_up() * radius);
		const Vec forward(origin - cinolib::GLcanvas::world_forward() * radius);
		const auto project{ [this](const Vec& _point) -> Vec {
			Vec proj(m_camera.projectionViewMatrix() * _point);
			proj.x() *= m_camera.projection.aspectRatio;
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