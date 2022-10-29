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
        m_centroid{}, m_kind{}, m_modifier{}, m_start{}, onPendingChanged{}, m_mouse{}
    {}

    void DirectVertEdit::request(EKind _kind, const Vec2& _mouse)
    {
        const bool wasPending{ m_pending };
        m_pending = false;
        m_vertEdit.applyAction();
        m_pending = !wasPending || m_kind != _kind;
        m_mouse = m_start = _mouse;
        m_modifier = EModifier::None;
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

    void DirectVertEdit::update(const Vec2& _mouse, EModifier _modifier)
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
        switch (m_kind)
        {
            case EKind::Translation:
            {
                m_modifier = _modifier == EModifier::XY ? EModifier::None : _modifier;
                const Vec2 diff{ m_mouse - m_start };
                const Vec2 newCentroid{ m_centroid + diff };
                const cinolib::Ray ray{ m_canvas.eye_to_screen_ray(newCentroid) };
                Real denom = forward.dot(ray.dir());
                if (!Utils::Transform::isNull(denom))
                {
                    const Real t = (transform.origin - ray.begin()).dot(forward) / denom;
                    const Vec newCentroid3d{ ray.begin() + ray.dir() * t };
                    if (t > 0)
                    {
                        transform.translation = newCentroid3d - transform.origin;
                    }
                }
            }
            break;
            case EKind::Scale:
            {
                m_modifier = _modifier;
                const Vec2 diff{ m_mouse - m_centroid };
                const Vec2 startDiff{ m_start - m_centroid };
                const auto scaleDim{ [&diff, &startDiff](unsigned int _dim) {
                    const Real dd{diff[_dim]}, dsd{startDiff[_dim]};
                    return Utils::Transform::isNull(dsd) ? 1.0 : std::abs(dd / dsd);
                } };
                Vec2 scale{ 1.0 };
                switch (m_modifier)
                {
                    case EModifier::None:
                        scale.x() = scaleDim(0);
                        scale.y() = scaleDim(1);
                        break;
                    case EModifier::X:
                        scale.x() = scaleDim(0);
                        break;
                    case EModifier::Y:
                        scale.y() = scaleDim(1);
                        break;
                    case EModifier::XY:
                    {
                        const Real dn{ diff.norm() }, dsn{ startDiff.norm() };
                        scale = { Utils::Transform::isNull(dsn) ? 1.0 : (dn / dsn) };
                    }
                    break;
                }
                transform.scale = right * scale.x() + up * scale.y();
                transform.scale.x() = std::abs(transform.scale.x());
                transform.scale.y() = std::abs(transform.scale.y());
            }
            break;
            case EKind::Rotation:
            {
                m_modifier = EModifier::None;
                const Vec2 startDir{ Utils::Transform::dir(m_centroid, m_start) };
                const Vec2 dir{ Utils::Transform::dir(m_centroid, m_mouse) };
                const Real angle{ Utils::Transform::angle(startDir, dir) };
                const Mat3 mat{ Utils::Transform::rotationMat(forward, angle) };
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

    DirectVertEdit::EModifier DirectVertEdit::modifier() const
    {
        ensurePending();
        return m_modifier;
    }

    void DirectVertEdit::draw()
    {
        if (!m_pending)
        {
            return;
        }
        using Utils::Controls::toImGui;
        ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
        const ImU32 startCol{ ImGui::ColorConvertFloat4ToU32(toImGui(c_lineStartColor)) };
        const ImU32 col{ ImGui::ColorConvertFloat4ToU32(toImGui(c_lineColor)) };
        const Real maxLen{ Vec2{static_cast<Real>(m_canvas.canvas_width()), static_cast<Real>(m_canvas.height())}.norm() };
        switch (m_kind)
        {
            case EKind::Rotation:
            {
                const Vec2 startDir{ Utils::Transform::dir(m_centroid, m_start) };
                const Vec2 dir{ Utils::Transform::dir(m_centroid, m_mouse) };
                drawList.AddLine(toImGui(m_centroid), toImGui(m_centroid + startDir * maxLen), startCol, c_lineThickness);
                drawList.AddLine(toImGui(m_centroid), toImGui(m_centroid + dir * maxLen), col, c_lineThickness);
            }
            break;
            case EKind::Scale:
            {
                static constexpr unsigned int segCount{ 36 };
                const Real startRadius{ m_centroid.dist(m_start) };
                const Real radius{ m_centroid.dist(m_mouse) };
                switch (m_modifier)
                {
                    case EModifier::X:
                        drawList.AddLine(toImGui(m_centroid - Vec2{ maxLen, 0 }), toImGui(m_centroid + Vec2{ maxLen, 0 }), startCol, c_lineThickness);
                        break;
                    case EModifier::Y:
                        drawList.AddLine(toImGui(m_centroid - Vec2{ 0, maxLen }), toImGui(m_centroid + Vec2{ 0, maxLen }), startCol, c_lineThickness);
                        break;
                    case EModifier::XY:
                        drawList.AddLine(toImGui(m_centroid - Vec2{ maxLen, -maxLen }), toImGui(m_centroid + Vec2{ maxLen, -maxLen }), startCol, c_lineThickness);
                        drawList.AddLine(toImGui(m_centroid - Vec2{ maxLen, maxLen }), toImGui(m_centroid + Vec2{ maxLen, maxLen }), startCol, c_lineThickness);
                        break;
                }
                drawList.AddCircle(toImGui(m_centroid), static_cast<float>(startRadius), startCol, segCount, c_lineThickness);
                drawList.AddCircle(toImGui(m_centroid), static_cast<float>(radius), col, segCount, c_lineThickness);
            }
            break;
            case EKind::Translation:
            {
                static constexpr Real crossRadius{ 10.0f };
                switch (m_modifier)
                {
                    case EModifier::X:
                        drawList.AddLine(toImGui(m_start - Vec2{ maxLen, 0 }), toImGui(m_start + Vec2{ maxLen, 0 }), startCol, c_lineThickness);
                        drawList.AddLine(toImGui(m_start - Vec2{ 0, crossRadius }), toImGui(m_start + Vec2{ 0, crossRadius }), startCol, c_lineThickness);
                        drawList.AddLine(toImGui(Vec2{ m_mouse.x(), m_start.y() } - Vec2{ 0, crossRadius }), toImGui(Vec2{ m_mouse.x(), m_start.y() } + Vec2{ 0, crossRadius }), col, c_lineThickness);
                        break;
                    case EModifier::Y:
                        drawList.AddLine(toImGui(m_start - Vec2{ 0, maxLen }), toImGui(m_start + Vec2{ 0, maxLen }), startCol, c_lineThickness);
                        drawList.AddLine(toImGui(m_start - Vec2{ crossRadius, 0 }), toImGui(m_start + Vec2{ crossRadius, 0 }), startCol, c_lineThickness);
                        drawList.AddLine(toImGui(Vec2{ m_start.x(), m_mouse.y() } - Vec2{ crossRadius, 0 }), toImGui(Vec2{ m_start.x(), m_mouse.y() } + Vec2{ crossRadius, 0 }), col, c_lineThickness);
                        break;
                    default:
                        drawList.AddLine(toImGui(m_start - Vec2{ crossRadius, 0 }), toImGui(m_start + Vec2{ crossRadius, 0 }), startCol, c_lineThickness);
                        drawList.AddLine(toImGui(m_start - Vec2{ 0, crossRadius }), toImGui(m_start + Vec2{ 0, crossRadius }), startCol, c_lineThickness);
                        drawList.AddLine(toImGui(m_start), toImGui(m_mouse), col, c_lineThickness);
                        break;
                }
            }
            break;
        }
    }

}