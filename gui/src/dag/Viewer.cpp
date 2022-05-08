#include <HMP/Gui/Dag/Viewer.hpp>

#include <HMP/Dag/Node.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/clamp.h>
#include <algorithm>
#include <cmath>
#include <cinolib/geometry/vec_mat_utils.h>
#include <imgui.h>
#include <string>
#include <limits>


namespace HMP::Gui::Dag
{

	using namespace HMP::Dag;

	Viewer::Viewer(const Meshing::Mesher& _mesher)
		: m_center_nl{ 0.5, 0.5 }, m_windowHeight_n{ 1.0 }, m_mesher{ _mesher }
	{}

	const Meshing::Mesher& Viewer::mesher() const
	{
		return m_mesher;
	}

	void Viewer::resetView()
	{
		m_center_nl = cinolib::vec2d{ layout.aspectRatio(), 1.0 } / 2;
		m_windowHeight_n = std::numeric_limits<double>::infinity();
	}

	void Viewer::draw()
	{

		// types

		using vec = cinolib::vec2d;

		constexpr auto toVec{ [](const ImVec2& _vec) {
			return vec{_vec.x, _vec.y};
		} };

		constexpr auto toImVec{ [](const vec& _vec) {
			return ImVec2{static_cast<float>(_vec.x()), static_cast<float>(_vec.y())};
		} };

		// window

		const vec windowSize_s{ toVec(ImGui::GetContentRegionAvail()) };
		if (windowSize_s.x() <= 0.0 || windowSize_s.y() <= 0.0)
		{
			return;
		}
		const vec topLeft_sw{ toVec(ImGui::GetCursorScreenPos()) };
		const vec bottomRight_sw{ topLeft_sw + windowSize_s };
		const double windowAspectRatio{ windowSize_s.x() / windowSize_s.y() };

		if (layout.size().x() <= 0.0 || layout.size().y() <= 0.0)
		{
			return;
		}

		// trasformations

		const double s2n{ 1.0 / windowSize_s.y() };
		const double n2s{ 1.0 / s2n };
		const double n2l{ layout.size().y() };
		const double l2n{ 1.0 / n2l };

		const auto ss2sw{ [&](const vec& _point_ss) {
			return _point_ss - topLeft_sw;
		} };

		const auto sw2ss{ [&](const vec& _point_sw) {
			return _point_sw + topLeft_sw;
		} };

		const auto sw2nw{ [&](const vec& _point_sw) {
			return vec{ _point_sw.x() * s2n, 1.0 - _point_sw.y() * s2n };
		} };

		const auto nw2sw{ [&](const vec& _point_nw) {
			return vec{ _point_nw.x(), 1.0 - _point_nw.y() } *n2s;
		} };

		const auto nw2nl{ [&](const vec& _point_nw) {
			return (_point_nw - vec{windowAspectRatio, 1} / 2) * m_windowHeight_n + m_center_nl;
		} };

		const auto nl2nw{ [&](const vec& _point_nl) {
			return (_point_nl - m_center_nl) / m_windowHeight_n + vec{windowAspectRatio, 1} / 2;
		} };

		const auto nl2ll{ [&](const vec& _point_nl) {
			return _point_nl * n2l + layout.bottomLeft();
		} };

		const auto ll2nl{ [&](const vec& _point_ll) {
			return (_point_ll - layout.bottomLeft()) * l2n;
		} };

		// input

		ImGuiIO& io = ImGui::GetIO();
		ImGui::InvisibleButton("canvas", toImVec(windowSize_s), ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

		const auto clampCenter{ [&]() {
			m_center_nl.x() = cinolib::clamp(m_center_nl.x(), 0.0, layout.aspectRatio());
			m_center_nl.y() = cinolib::clamp(m_center_nl.y(), 0.0, 1.0);
		} };

		const auto clampHeight{ [&]() {
			double min{ 0.1 }, max{ 2.0 };
			if (windowAspectRatio < layout.aspectRatio())
			{
				const double factor{ layout.aspectRatio() / windowAspectRatio };
				min *= factor;
				max *= factor;
			}
			m_windowHeight_n = cinolib::clamp(m_windowHeight_n, min, max);
		} };

		clampHeight();

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f))
		{
			m_center_nl -= vec{ io.MouseDelta.x, -io.MouseDelta.y } *s2n * m_windowHeight_n;
		}

		clampCenter();

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
		{
			constexpr double speed{ 2 };
			const vec mouse_ss{ toVec(io.MouseClickedPos[ImGuiMouseButton_Middle]) };
			const vec oldMouse_nl{ nw2nl(sw2nw(ss2sw(mouse_ss))) };
			m_windowHeight_n += io.MouseDelta.y * s2n * m_windowHeight_n * speed;
			clampHeight();
			const vec newMouse_nl{ nw2nl(sw2nw(ss2sw(mouse_ss))) };
			m_center_nl += oldMouse_nl - newMouse_nl;
			clampCenter();
		}

		// drawing

		constexpr auto toImCol{ [](const cinolib::Color& _color) {
			return IM_COL32(_color.r_uchar(), _color.g_uchar(), _color.b_uchar(), _color.a_uchar());
		} };

		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			constexpr ImU32 backgroundColor{ IM_COL32(40, 40, 40, 255) };
			constexpr ImU32 borderColor{ IM_COL32(255, 255, 255, 255) };

			drawList->AddRectFilled(toImVec(topLeft_sw), toImVec(bottomRight_sw), backgroundColor);
			drawList->AddRect(toImVec(topLeft_sw), toImVec(bottomRight_sw), borderColor);

			drawList->PushClipRect(toImVec(topLeft_sw + vec{ 1,1 }), toImVec(bottomRight_sw - vec{ 1,1 }), true);
			{
				const auto ll2ss{ [&](const vec& _point_ll) {
					return sw2ss(nw2sw(nl2nw(ll2nl(_point_ll))));
				} };

				const double l2s{ l2n / m_windowHeight_n * n2s };

				// grid

				{
					constexpr ImU32 gridColor{ IM_COL32(60, 60, 60, 255) };
					const int gridLevel{ std::max(0, static_cast<int>(-std::log2(m_windowHeight_n / 2.0))) };
					const double gridStep_s{ n2s / ((1 << gridLevel) * 10) / m_windowHeight_n };
					const vec origin_ss{ sw2ss(nw2sw(nl2nw(vec{0,1}))) };
					for (double x_s{ std::fmod(origin_ss.x(), gridStep_s) }; x_s <= bottomRight_sw.x(); x_s += gridStep_s)
					{
						drawList->AddLine(toImVec(vec{ x_s, topLeft_sw.y() }), toImVec(vec{ x_s, bottomRight_sw.y() }), gridColor);
					}
					for (double y_s{ std::fmod(origin_ss.y(), gridStep_s) }; y_s <= bottomRight_sw.y(); y_s += gridStep_s)
					{
						drawList->AddLine(toImVec(vec{ topLeft_sw.x(), y_s }), toImVec(vec{ bottomRight_sw.x(), y_s }), gridColor);
					}
				}

				// edges

				constexpr ImU32 strokeColor{ IM_COL32(220, 220, 220, 255) };

				for (const Layout::Line& line : layout.lines())
				{
					drawList->AddLine(toImVec(ll2ss(line.first)), toImVec(ll2ss(line.second)), strokeColor);
				}

				// nodes

				const float nodeRadius_s{ static_cast<float>(layout.nodeRadius() * l2s) };
				const vec nodeHalfDiag_s{ nodeRadius_s, nodeRadius_s };

				for (const Layout::Node& node : layout.nodes())
				{
					constexpr ImU32 textColor{ backgroundColor };
					const vec center{ ll2ss(node.center()) };
					std::string text{};
					switch (node.node().type())
					{
						case Dag::Node::EType::Element:
						{
							static const ImU32 elementColor{ toImCol(Meshing::Mesher::polyColor) };
							static const ImU32 highlightedElementColor{ toImCol(Meshing::Mesher::markedFaceColor) };
							const ImU32 color{ highlight == &node.node() ? highlightedElementColor : elementColor };
							drawList->AddRectFilled(toImVec(center - nodeHalfDiag_s), toImVec(center + nodeHalfDiag_s), color);
							drawList->AddRect(toImVec(center - nodeHalfDiag_s), toImVec(center + nodeHalfDiag_s), strokeColor);
							const Dag::Element& element{ node.node().element() };
							text = m_mesher.has(element) ? std::to_string(m_mesher.elementToPid(element)) : "";
						}
						break;
						case Dag::Node::EType::Operation:
						{
							constexpr int circleSegments{ 10 };
							ImU32 operationColor{};
							switch (node.node().operation().primitive())
							{
								case Dag::Operation::EPrimitive::Extrude:
									text = "E";
									operationColor = IM_COL32(252, 109, 171, 255);
									break;
								case Dag::Operation::EPrimitive::Refine:
									text = "R";
									operationColor = IM_COL32(192, 76, 253, 255);
									break;
								case Dag::Operation::EPrimitive::Delete:
									text = "D";
									operationColor = IM_COL32(94, 43, 255, 255);
									break;
							}
							drawList->AddCircleFilled(toImVec(center), nodeRadius_s, operationColor, circleSegments);
							drawList->AddCircle(toImVec(center), nodeRadius_s, strokeColor, circleSegments);
						}
						break;
					}
					const vec textSize{ toVec(ImGui::CalcTextSize(text.c_str())) / ImGui::GetFontSize() * nodeRadius_s };
					drawList->AddText(
						ImGui::GetFont(),
						nodeRadius_s,
						toImVec(center - textSize / 2),
						textColor,
						text.c_str());
				}
			}
			drawList->PopClipRect();
		}

	}

}