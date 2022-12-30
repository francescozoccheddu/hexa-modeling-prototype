#include <HMP/Gui/DagViewer/Widget.hpp>

#include <HMP/Dag/Node.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/clamp.h>
#include <algorithm>
#include <cmath>
#include <cinolib/geometry/vec_mat_utils.h>
#include <cinolib/fonts/droid_sans.hpp>
#include <imgui.h>
#include <imgui_impl_opengl2.h>
#include <string>
#include <HMP/Gui/Utils/HrDescriptions.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <limits>
#include <chrono>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/themer.hpp>

namespace HMP::Gui::DagViewer
{

	void Widget::initFonts()
	{
		static constexpr float c_minFontSize{ 100.0f };
		ImGuiIO& io{ ImGui::GetIO() };
		ImFont** maxFontIt{ std::max_element(io.Fonts->Fonts.begin(), io.Fonts->Fonts.end(), [](ImFont* _a, ImFont* _b) { return _a->FontSize < _b->FontSize; }) };
		if (maxFontIt == io.Fonts->Fonts.end() || (*maxFontIt)->FontSize < c_minFontSize)
		{
			ImVector<ImWchar> ranges{};
			ImFontGlyphRangesBuilder builder{};
			builder.AddText("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-?");
			builder.BuildRanges(&ranges);
			ImFontConfig config{};
			io.Fonts->AddFontFromMemoryCompressedTTF(cinolib::droid_sans_data, static_cast<int>(cinolib::droid_sans_size), c_minFontSize, &config, ranges.Data);
			io.Fonts->Build();
			ImGui_ImplOpenGL2_DestroyFontsTexture();
			ImGui_ImplOpenGL2_CreateFontsTexture();
		}
	}

	Widget::Widget(cpputils::collections::Namer<const Dag::Node*>& _namer)
		: cinolib::SideBarItem{ "Dag" }, m_namer{ _namer }, m_center_nl{ 0.5, 0.5 }, m_windowHeight_n{ 1.0 }
	{
		initFonts();
	}

	const cpputils::collections::Namer<const Dag::Node*>& Widget::namer() const
	{
		return m_namer;
	}

	Layout& Widget::layout()
	{
		return m_layout;
	}

	const Layout& Widget::layout() const
	{
		return m_layout;
	}

	void Widget::resetView()
	{
		m_center_nl = Vec2{ m_layout.aspectRatio(), 1.0 } / 2;
		m_windowHeight_n = std::numeric_limits<Real>::infinity();
	}

	void Widget::draw()
	{

		{
			const auto t1{ std::chrono::high_resolution_clock::now() };
			onDraw();
			const auto t2{ std::chrono::high_resolution_clock::now() };
			const int64_t elapsedMs{ std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() };
			static constexpr int64_t warningTimeThresholdMs{ 200 };
			if (elapsedMs >= warningTimeThresholdMs)
			{
				showLayoutPerformanceWarning = true;
			}
		}

		if (showLayoutPerformanceWarning)
		{
			ImGui::TextColored(themer->sbWarn, "Leaving this widget open will affect meshing performance!");
		}

		// types

		static constexpr auto toVec{ [](const ImVec2& _vec) {
			return Vec2{_vec.x, _vec.y};
		} };

		static constexpr auto toImVec{ [](const Vec2& _vec) {
			return ImVec2{static_cast<float>(_vec.x()), static_cast<float>(_vec.y())};
		} };

		// window

		const ImGuiStyle& style{ ImGui::GetStyle() };

		const Vec2 availWindowSize_s{ toVec(ImGui::GetContentRegionAvail()) };
		const Vec2 windowSize_s{ availWindowSize_s.x(), std::max(availWindowSize_s.x(), availWindowSize_s.y() - static_cast<Real>(style.ItemSpacing.y)) };
		if (windowSize_s.x() <= 0.0 || windowSize_s.y() <= 0.0)
		{
			return;
		}
		const Vec2 topLeft_sw{ toVec(ImGui::GetCursorScreenPos()) };
		const Vec2 bottomRight_sw{ topLeft_sw + windowSize_s };
		const Real windowAspectRatio{ windowSize_s.x() / windowSize_s.y() };

		if (m_layout.size().x() <= 0.0 || m_layout.size().y() <= 0.0)
		{
			return;
		}

		// trasformations

		const Real s2n{ 1.0 / windowSize_s.y() };
		const Real n2s{ 1.0 / s2n };
		const Real n2l{ m_layout.size().y() };
		const Real l2n{ 1.0 / n2l };

		const auto ss2sw{ [&](const Vec2& _point_ss) {
			return _point_ss - topLeft_sw;
		} };

		const auto sw2ss{ [&](const Vec2& _point_sw) {
			return _point_sw + topLeft_sw;
		} };

		const auto sw2nw{ [&](const Vec2& _point_sw) {
			return Vec2{ _point_sw.x() * s2n, 1.0 - _point_sw.y() * s2n };
		} };

		const auto nw2sw{ [&](const Vec2& _point_nw) {
			return Vec2{ _point_nw.x(), 1.0 - _point_nw.y() } *n2s;
		} };

		const auto nw2nl{ [&](const Vec2& _point_nw) {
			return (_point_nw - Vec2{windowAspectRatio, 1} / 2) * m_windowHeight_n + m_center_nl;
		} };

		const auto nl2nw{ [&](const Vec2& _point_nl) {
			return (_point_nl - m_center_nl) / m_windowHeight_n + Vec2{windowAspectRatio, 1} / 2;
		} };

		/*const auto nl2ll{ [&](const Vec2& _point_nl) {
			return _point_nl * n2l + m_layout.bottomLeft();
		} };*/

		const auto ll2nl{ [&](const Vec2& _point_ll) {
			return (_point_ll - m_layout.bottomLeft()) * l2n;
		} };

		// font

		ImGuiIO& io = ImGui::GetIO();

		ImFont** maxFontIt{ std::max_element(io.Fonts->Fonts.begin(), io.Fonts->Fonts.end(), [](ImFont* _a, ImFont* _b) { return _a->FontSize < _b->FontSize; }) };
		ImGui::PushFont(*maxFontIt);

		// input

		ImGui::InvisibleButton("canvas", toImVec(windowSize_s), ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);

		const auto clampCenter{ [&]() {
			m_center_nl.x() = cinolib::clamp(m_center_nl.x(), 0.0, m_layout.aspectRatio());
			m_center_nl.y() = cinolib::clamp(m_center_nl.y(), 0.0, 1.0);
		} };

		const auto clampHeight{ [&]() {
			const Real fullHeight_n {(windowAspectRatio < m_layout.aspectRatio()) ? m_layout.aspectRatio() / windowAspectRatio : 1};
			const Real min{ m_layout.nodeRadius() * l2n * 3 };
			const Real max{ fullHeight_n * 1.1 + m_layout.nodeRadius() * l2n };
			m_windowHeight_n = cinolib::clamp(m_windowHeight_n, min, max);
		} };

		clampHeight();

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.0f))
		{
			m_center_nl -= Vec2{ io.MouseDelta.x, -io.MouseDelta.y } *s2n * m_windowHeight_n;
		}

		clampCenter();

		if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f))
		{
			constexpr Real speed{ 2 };
			const Vec2 mouse_ss{ toVec(io.MouseClickedPos[ImGuiMouseButton_Middle]) };
			const Vec2 oldMouse_nl{ nw2nl(sw2nw(ss2sw(mouse_ss))) };
			m_windowHeight_n += io.MouseDelta.y * s2n * m_windowHeight_n * speed;
			clampHeight();
			const Vec2 newMouse_nl{ nw2nl(sw2nw(ss2sw(mouse_ss))) };
			m_center_nl += oldMouse_nl - newMouse_nl;
			clampCenter();
		}

		// drawing

		{
			ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
			const ImU32 borderColor{ ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_Border]) };
			const ImU32 gridColor{ ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_TableBorderLight]) };
			const ImU32 strokeColor{ ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_PlotLines]) };
			const ImU32 backgroundColor{ ImGui::ColorConvertFloat4ToU32(style.Colors[ImGuiCol_PopupBg]) };

			drawList.AddRectFilled(toImVec(topLeft_sw), toImVec(bottomRight_sw), backgroundColor);
			drawList.AddRect(toImVec(topLeft_sw), toImVec(bottomRight_sw), borderColor);

			drawList.PushClipRect(toImVec(topLeft_sw + Vec2{ 1,1 }), toImVec(bottomRight_sw - Vec2{ 1,1 }), true);
			{
				const auto ll2ss{ [&](const Vec2& _point_ll) {
					return sw2ss(nw2sw(nl2nw(ll2nl(_point_ll))));
				} };

				const Real l2s{ l2n / m_windowHeight_n * n2s };

				// grid

				{
					const int gridLevel{ static_cast<int>(-std::log2(m_windowHeight_n / 2.0)) };
					const Real gridStep_s{ n2s / (std::pow(2, gridLevel) * 10) / m_windowHeight_n };
					const Vec2 origin_ss{ sw2ss(nw2sw(nl2nw(Vec2{0,1}))) };
					for (Real x_s{ std::fmod(origin_ss.x(), gridStep_s) }; x_s <= bottomRight_sw.x(); x_s += gridStep_s)
					{
						drawList.AddLine(toImVec(Vec2{ x_s, topLeft_sw.y() }), toImVec(Vec2{ x_s, bottomRight_sw.y() }), gridColor);
					}
					for (Real y_s{ std::fmod(origin_ss.y(), gridStep_s) }; y_s <= bottomRight_sw.y(); y_s += gridStep_s)
					{
						drawList.AddLine(toImVec(Vec2{ topLeft_sw.x(), y_s }), toImVec(Vec2{ bottomRight_sw.x(), y_s }), gridColor);
					}
				}

				// edges

				for (const auto& [lineA, lineB] : m_layout.lines())
				{
					drawList.AddLine(toImVec(ll2ss(lineA)), toImVec(ll2ss(lineB)), strokeColor);
				}

				// nodes

				const float nodeRadius_s{ static_cast<float>(m_layout.nodeRadius() * l2s) };
				const float copiedNodeRadius_s{ static_cast<float>(m_layout.nodeRadius() * l2s) * 1.1f };
				const Vec2 nodeHalfDiag_s{ nodeRadius_s, nodeRadius_s };
				const Vec2 copiedNodeHalfDiag_s{ copiedNodeRadius_s, copiedNodeRadius_s };

				for (const Layout::Node& node : m_layout.nodes())
				{
					const Vec2 center{ ll2ss(node.center()) };
					switch (node.node().type)
					{
						case Dag::Node::EType::Element:
						{
							const Dag::Element& element{ node.node().element() };
							const ImU32 color{ highlight == &node.node()
								? themer->dagNodeElHi
								: Meshing::Utils::isShown(element) ? themer->dagNodeEl : themer->dagNodeElMut };
							drawList.AddRectFilled(toImVec(center - nodeHalfDiag_s), toImVec(center + nodeHalfDiag_s), color);
							drawList.AddRect(toImVec(center - nodeHalfDiag_s), toImVec(center + nodeHalfDiag_s), strokeColor);
							if (&element == copied)
							{
								drawList.AddRect(toImVec(center - copiedNodeHalfDiag_s), toImVec(center + copiedNodeHalfDiag_s), strokeColor);
							}
						}
						break;
						case Dag::Node::EType::Operation:
						{
							constexpr int circleSegments{ 10 };
							ImU32 operationColor{};
							switch (node.node().operation().primitive)
							{
								case Dag::Operation::EPrimitive::Extrude:
									operationColor = themer->dagNodeExtrude;
									break;
								case Dag::Operation::EPrimitive::Refine:
									operationColor = themer->dagNodeRefine;
									break;
								case Dag::Operation::EPrimitive::Delete:
									operationColor = themer->dagNodeDelete;
									break;
							}
							drawList.AddCircleFilled(toImVec(center), nodeRadius_s, operationColor, circleSegments);
							drawList.AddCircle(toImVec(center), nodeRadius_s, strokeColor, circleSegments);
						}
						break;
					}
					const std::string text{ Utils::HrDescriptions::name(node.node(), m_namer) };
					const Vec2 textSize{ toVec(ImGui::CalcTextSize(text.c_str())) / ImGui::GetFontSize() };
					const Real maxTextSize{ std::max(textSize.x(), textSize.y()) };
					const Real fontSize{ nodeRadius_s * 1.25 / maxTextSize };
					Utils::Drawing::text(drawList, text.c_str(), toImVec(center), static_cast<float>(fontSize), backgroundColor);
				}
			}
			drawList.PopClipRect();
		}

		ImGui::PopFont();
	}

}