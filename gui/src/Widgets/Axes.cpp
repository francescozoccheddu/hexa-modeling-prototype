#include <HMP/Gui/Widgets/Axes.hpp>

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <cinolib/deg_rad.h>
#include <cinolib/gl/glcanvas.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <cmath>
#include <array>
#include <utility>
#include <algorithm>

namespace HMP::Gui::Widgets
{

	void Axes::draw(const cinolib::GLcanvas& _canvas)
	{
		using Utils::Controls::toImGui;
		ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
		Vec origin;
		Real radius;
		const cinolib::FreeCamera<Real>& camera{ _canvas.camera };
		if (camera.projection.perspective)
		{
			origin = camera.view.centerAt(3);
			radius = std::tan(cinolib::to_rad(camera.projection.verticalFieldOfView / 2));
		}
		else
		{
			origin = camera.view.centerAt(2);
			radius = camera.projection.verticalFieldOfView / 2 * 0.75;
		}
		const Vec right(origin + cinolib::GLcanvas::world_right() * radius);
		const Vec up(origin + cinolib::GLcanvas::world_up() * radius);
		const Vec forward(origin - cinolib::GLcanvas::world_forward() * radius);
		const Real maxSize{ static_cast<Real>(std::min(_canvas.canvas_width(), _canvas.height())) };
		const Real size{ std::min((maxSize * 0.1 + 100) / 2, maxSize / 3) };
		const auto project{ [&](const Vec& _point) -> Vec {
			Vec proj(camera.projectionViewMatrix() * _point);
			proj.x() *= camera.projection.aspectRatio;
			const ImVec2 windowOrigin{ ImGui::GetWindowPos() };
			const ImVec2 windowSize{ ImGui::GetWindowSize() };
			proj.x() = proj.x() * size + windowOrigin.x + windowSize.x - size;
			proj.y() = -proj.y() * size + windowOrigin.y + windowSize.y - size;
			return proj;
		} };
		origin = project(origin);
#ifdef HMP_GUI_LIGHT_THEME
#define HMP_GUI_WIDGETS_AXES_COL_VAL 240
#define HMP_GUI_WIDGETS_AXES_COL_NVAL 100
#else
#define HMP_GUI_WIDGETS_AXES_COL_VAL 255
#define HMP_GUI_WIDGETS_AXES_COL_NVAL 100
#endif
		std::array<std::pair<Vec, ImColor>, 3> tips{
			std::pair<Vec, ImColor>{project(right), IM_COL32(HMP_GUI_WIDGETS_AXES_COL_VAL,HMP_GUI_WIDGETS_AXES_COL_NVAL,HMP_GUI_WIDGETS_AXES_COL_NVAL,255)},
			std::pair<Vec, ImColor>{project(up), IM_COL32(HMP_GUI_WIDGETS_AXES_COL_NVAL,HMP_GUI_WIDGETS_AXES_COL_VAL,HMP_GUI_WIDGETS_AXES_COL_NVAL,255)},
			std::pair<Vec, ImColor>{project(forward), IM_COL32(HMP_GUI_WIDGETS_AXES_COL_NVAL,HMP_GUI_WIDGETS_AXES_COL_NVAL,HMP_GUI_WIDGETS_AXES_COL_VAL,255)}
		};
#undef HMP_GUI_WIDGETS_AXES_COL_VAL
#undef HMP_GUI_WIDGETS_AXES_COL_NVAL
		std::sort(tips.begin(), tips.end(), [](const std::pair<Vec, ImColor>& _a, const std::pair<Vec, ImColor>& _b) { return _a.first.z() > _b.first.z(); });
		for (const auto& [tip, color] : tips)
		{
			Utils::Drawing::line(drawList, toImGui(origin.rem_coord()), toImGui(tip.rem_coord()), color, 3.0f);
			Utils::Drawing::circleFilled(drawList, toImGui(tip.rem_coord()), 5.0f, color);
		}
		}

	}