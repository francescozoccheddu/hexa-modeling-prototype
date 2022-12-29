#include <HMP/Gui/Utils/Theme.hpp>

namespace HMP::Gui::Utils
{

    void Theme::applyImGui() const
    {
        static constexpr float lightStyleHue{ 213.0f / 360.0f };
        static constexpr float darkStyleHue{ 213.0f / 360.0f };
        const float styleHue{ dark ? darkStyleHue : lightStyleHue };
        if (dark)
        {
            ImGui::StyleColorsDark();
        }
        else
        {
            ImGui::StyleColorsLight();
        }
        ImGuiStyle& style{ ImGui::GetStyle() };
        for (ImVec4& color : style.Colors)
        {
            float h, s, v;
            ImGui::ColorConvertRGBtoHSV(color.x, color.y, color.z, h, s, v);
            h += hue - styleHue;
            h = static_cast<float>(Utils::Transform::wrapAngle(static_cast<Real>(h * 360.0f))) / 360.0f;
            ImGui::ColorConvertHSVtoRGB(h, s, v, color.x, color.y, color.z);
        }
    }

    void Theme::apply(Meshing::Mesher& _mesher) const
    {
        _mesher.faceColor = faceColor;
        _mesher.edgeColor = edgeColor;
        _mesher.updateColors();
    }

    void Theme::apply(Widgets::Axes& _widget) const
    {
        _widget.colorSat = axesColorSat;
        _widget.colorVal = axesColorVal;
    }

    void Theme::apply(Widgets::DirectVertEdit& _widget) const
    {
        _widget.color = overlayColor;
        _widget.mutedColor = mutedOverlayColor;
    }

    void Theme::apply(Widgets::Target& _widget) const
    {
        _widget.edgeColor = targetEdgeColor;
        _widget.faceColor = targetFaceColor;
        if (_widget.hasMesh())
        {
            _widget.updateColor();
        }
    }

    void Theme::apply(Widgets::VertEdit& _widget) const
    {
        _widget.color = overlayColor;
    }

    void Theme::apply(App& _app) const
    {
        _app.overlayColor = overlayColor;
        _app.mutedOverlayColor = mutedOverlayColor;
        _app.warningTextColor = warningTextColor;
        _app.highlightedFaceColor = highlightedFaceColor;
        _app.highlightedPolyColor = highlightedPolyColor;
    }

    void Theme::apply(cinolib::GLcanvas& _canvas) const
    {
        _canvas.background = backgroundColor;
    }

    void Theme::apply(Widgets::Debug& _widget) const
    {
        _widget.nameColor = overlayColor;
    }

#ifdef HMP_GUI_ENABLE_DAG_VIEWER

    void Theme::apply(DagViewer::Widget& _widget) const
    {
        _widget.elementColor = dagElementColor;
        _widget.highlightedElementColor = dagHighlightedElementColor;
    }

#endif

}