#include <HMP/Gui/Widgets/DirectVertEdit.hpp>

#include <HMP/Gui/Utils/Transform.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <stdexcept>
#include <imgui.h>

namespace HMP::Gui::Widgets
{

    void DirectVertEdit::ensurePending() const
    {
        if (!m_pending)
        {
            throw std::logic_error{ "not pending" };
        }
    }

    DirectVertEdit::DirectVertEdit(VertEdit& _vertEdit, const cinolib::GLcanvas& _canvas) :
        m_vertEdit{ _vertEdit }, m_canvas{ _canvas }, m_pending{ false },
        m_centroid{}, m_kind{}, m_onX{}, m_onY{}, m_onZ{}, m_start{}, onPendingChanged{}, m_mouse{}
    {}

    void DirectVertEdit::request(EKind _kind, const Vec2& _mouse)
    {
        if (m_vertEdit.empty() && !m_pending)
        {
            return;
        }
        const bool wasPending{ m_pending };
        m_pending = false;
        m_vertEdit.applyAction();
        m_pending = !wasPending || m_kind != _kind;
        m_mouse = m_start = _mouse;
        m_onX = m_onY = m_onZ = false;
        GLdouble depth;
        m_canvas.project(m_vertEdit.centroid(), m_centroid, depth);
        m_kind = _kind;
        Utils::Transform& transform{ m_vertEdit.transform() };
        transform.translation = { 0.0 };
        transform.scale = { 1.0 };
        transform.rotation = { 0.0 };
        m_vertEdit.applyTransform();
        if (wasPending != m_pending)
        {
            onPendingChanged();
        }
    }

    void DirectVertEdit::update(const Vec2& _mouse, bool _onX, bool _onY, bool _onZ)
    {
        if (!m_pending)
        {
            return;
        }
        m_mouse = _mouse;
        const Vec up{ m_canvas.camera.view.normUp() };
        const Vec right{ m_canvas.camera.view.normRight() };
        const Vec forward{ m_canvas.camera.view.normBack() };
        Utils::Transform& transform{ m_vertEdit.transform() };
        transform.translation = { 0.0 };
        transform.scale = { 1.0 };
        transform.rotation = { 0.0 };
        m_onX = _onX;
        m_onY = _onY;
        m_onZ = _onZ;
        switch (m_kind)
        {
            case EKind::Translation:
            {
                if (m_onZ || (m_onX && m_onY))
                {
                    break;
                }
                Vec2 diff{ m_mouse - m_start };
                if (m_onY)
                {
                    diff.x() = 0.0;
                }
                if (m_onX)
                {
                    diff.y() = 0.0;
                }
                const Vec2 newCentroid{ m_centroid + diff };
                const cinolib::Ray ray{ m_canvas.eye_to_screen_ray(newCentroid) };
                Real denom = forward.dot(ray.dir());
                if (!Utils::Transform::isNull(denom))
                {
                    const Real t = (transform.origin - ray.begin()).dot(forward) / denom;
                    const Vec newCentroid3d{ ray.begin() + ray.dir() * t };
                    transform.translation = newCentroid3d - transform.origin;
                }
            }
            break;
            case EKind::Scale:
            {
                const Vec2 diff{ m_mouse - m_centroid };
                const Vec2 startDiff{ m_start - m_centroid };
                Vec scale{ 1.0 };
                if (!m_onX && !m_onY && !m_onZ)
                {
                    const auto scaleDim{ [&diff, &startDiff](unsigned int _dim) {
                        const Real dd{diff[_dim]}, dsd{startDiff[_dim]};
                        return Utils::Transform::isNull(dsd) ? 1.0 : std::abs(dd / dsd);
                    } };
                    transform.scale = right * scaleDim(0) + up * scaleDim(1);
                    transform.scale.x() = std::abs(transform.scale.x());
                    transform.scale.y() = std::abs(transform.scale.y());
                    transform.scale.z() = std::abs(transform.scale.z());
                }
                else
                {
                    const Real dn{ diff.norm() }, dsn{ startDiff.norm() };
                    const Real scale{ Utils::Transform::isNull(dsn) ? 1.0 : (dn / dsn) };
                    if (m_onX)
                    {
                        transform.scale.x() = scale;
                    }
                    if (m_onY)
                    {
                        transform.scale.y() = scale;
                    }
                    if (m_onZ)
                    {
                        transform.scale.z() = scale;
                    }
                }
            }
            break;
            case EKind::Rotation:
            {
                Vec axis{};
                if (!m_onX && !m_onY && !m_onZ)
                {
                    axis = forward;
                }
                else if (m_onX && !m_onY && !m_onZ)
                {
                    axis = { 1,0,0 };
                }
                else if (!m_onX && m_onY && !m_onZ)
                {
                    axis = { 0,1,0 };
                }
                else if (!m_onX && !m_onY && m_onZ)
                {
                    axis = { 0,0,1 };
                }
                else
                {
                    break;
                }
                if (axis.dot(forward) < 0)
                {
                    axis = -axis;
                }
                const Vec2 startDir{ Utils::Transform::dir(m_centroid, m_start) };
                const Vec2 dir{ Utils::Transform::dir(m_centroid, m_mouse) };
                const Real angle{ Utils::Transform::angle(startDir, dir) };
                const Mat3 mat{ Utils::Transform::rotationMat(axis, angle) };
                transform.rotation = Utils::Transform::rotationMatToVec(mat);
            }
            break;
        }
        m_vertEdit.applyTransform();
    }

    void DirectVertEdit::apply()
    {
        if (!m_pending)
        {
            return;
        }
        m_pending = false;
        m_vertEdit.applyAction();
        onPendingChanged();
    }

    void DirectVertEdit::cancel()
    {
        if (!m_pending)
        {
            return;
        }
        m_pending = false;
        m_vertEdit.cancel();
        onPendingChanged();
    }

    bool DirectVertEdit::pending() const
    {
        return m_pending;
    }

    DirectVertEdit::EKind DirectVertEdit::kind() const
    {
        ensurePending();
        return m_kind;
    }

    bool DirectVertEdit::locked() const
    {
        return m_onX || m_onY || m_onZ;
    }

    bool DirectVertEdit::onX() const
    {
        return m_onX;
    }

    bool DirectVertEdit::onY() const
    {
        return m_onY;
    }

    bool DirectVertEdit::onZ() const
    {
        return m_onZ;
    }

    void DirectVertEdit::draw()
    {
        if (!m_pending)
        {
            return;
        }
        using Utils::Controls::toImGui;
        ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
        const ImU32 startCol{ ImGui::ColorConvertFloat4ToU32(toImGui(c_mutedColor)) };
        const ImU32 col{ ImGui::ColorConvertFloat4ToU32(toImGui(c_color)) };
        const Real maxLen{ Vec2{static_cast<Real>(m_canvas.canvas_width()), static_cast<Real>(m_canvas.height())}.norm() };
        const ImFont* const font{ ImGui::GetFont() };
        switch (m_kind)
        {
            case EKind::Rotation:
            {
                const char* axisStr;
                if (!m_onX && !m_onY && !m_onZ)
                {
                    axisStr = "";
                }
                else if (m_onX && !m_onY && !m_onZ)
                {
                    axisStr = "X";
                }
                else if (!m_onX && m_onY && !m_onZ)
                {
                    axisStr = "Y";
                }
                else if (!m_onX && !m_onY && m_onZ)
                {
                    axisStr = "Z";
                }
                else
                {
                    drawList.AddText(font, c_textSize, toImGui(m_centroid + c_textMargin), col, "???");
                    break;
                }
                drawList.AddText(font, c_textSize, toImGui(m_centroid + c_textMargin), startCol, axisStr);
                const Vec2 startDir{ Utils::Transform::dir(m_centroid, m_start) };
                const Vec2 dir{ Utils::Transform::dir(m_centroid, m_mouse) };
                drawList.AddLine(toImGui(m_centroid), toImGui(m_centroid + startDir * maxLen), startCol, c_lineThickness);
                drawList.AddLine(toImGui(m_centroid), toImGui(m_centroid + dir * maxLen), col, c_lineThickness);
            }
            break;
            case EKind::Scale:
            {
                static constexpr unsigned int segCount{ 36 };
                if (m_onX || m_onY || m_onZ)
                {
                    std::string axisStr{};
                    if (m_onX) { axisStr += "X"; }
                    if (m_onY) { axisStr += "Y"; }
                    if (m_onZ) { axisStr += "Z"; }
                    drawList.AddText(font, c_textSize, toImGui(m_centroid + c_textMargin), startCol, axisStr.c_str());
                }
                const Real startRadius{ m_centroid.dist(m_start) };
                const Real radius{ m_centroid.dist(m_mouse) };
                drawList.AddCircle(toImGui(m_centroid), static_cast<float>(startRadius), startCol, segCount, c_lineThickness);
                drawList.AddCircle(toImGui(m_centroid), static_cast<float>(radius), col, segCount, c_lineThickness);
            }
            break;
            case EKind::Translation:
            {
                static constexpr Real crossRadius{ 10.0f };
                if (m_onX && !m_onY && !m_onZ)
                {
                    drawList.AddLine(toImGui(m_start - Vec2{ maxLen, 0 }), toImGui(m_start + Vec2{ maxLen, 0 }), startCol, c_lineThickness);
                    drawList.AddLine(toImGui(m_start - Vec2{ 0, crossRadius }), toImGui(m_start + Vec2{ 0, crossRadius }), startCol, c_lineThickness);
                    drawList.AddLine(toImGui(Vec2{ m_mouse.x(), m_start.y() } - Vec2{ 0, crossRadius }), toImGui(Vec2{ m_mouse.x(), m_start.y() } + Vec2{ 0, crossRadius }), col, c_lineThickness);
                    drawList.AddText(font, c_textSize, toImGui(m_start + c_textMargin), startCol, "X");
                }
                else if (!m_onX && m_onY && !m_onZ)
                {
                    drawList.AddLine(toImGui(m_start - Vec2{ 0, maxLen }), toImGui(m_start + Vec2{ 0, maxLen }), startCol, c_lineThickness);
                    drawList.AddLine(toImGui(m_start - Vec2{ crossRadius, 0 }), toImGui(m_start + Vec2{ crossRadius, 0 }), startCol, c_lineThickness);
                    drawList.AddLine(toImGui(Vec2{ m_start.x(), m_mouse.y() } - Vec2{ crossRadius, 0 }), toImGui(Vec2{ m_start.x(), m_mouse.y() } + Vec2{ crossRadius, 0 }), col, c_lineThickness);
                    drawList.AddText(font, c_textSize, toImGui(m_start + c_textMargin), startCol, "Y");
                }
                else if (!m_onX && !m_onY && !m_onZ)
                {
                    drawList.AddLine(toImGui(m_start - Vec2{ crossRadius, 0 }), toImGui(m_start + Vec2{ crossRadius, 0 }), startCol, c_lineThickness);
                    drawList.AddLine(toImGui(m_start - Vec2{ 0, crossRadius }), toImGui(m_start + Vec2{ 0, crossRadius }), startCol, c_lineThickness);
                    drawList.AddLine(toImGui(m_start), toImGui(m_mouse), col, c_lineThickness);
                }
                else
                {
                    drawList.AddLine(toImGui(m_start - Vec2{ crossRadius, 0 }), toImGui(m_start + Vec2{ crossRadius, 0 }), startCol, c_lineThickness);
                    drawList.AddLine(toImGui(m_start - Vec2{ 0, crossRadius }), toImGui(m_start + Vec2{ 0, crossRadius }), startCol, c_lineThickness);
                    drawList.AddText(font, c_textSize, toImGui(m_start + c_textMargin), col, "???");
                }
            }
            break;
        }
    }

}