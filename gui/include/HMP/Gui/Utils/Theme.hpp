#pragma once

#include <cinolib/color.h>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Gui/Widgets/Axes.hpp>
#include <HMP/Gui/Widgets/DirectVertEdit.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/App.hpp>
#include <cinolib/gl/glcanvas.h>
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
#include <HMP/Gui/DagViewer/Widget.hpp>
#endif

namespace HMP::Gui::Utils
{

    struct Theme final
    {

        float hue;
        bool dark;
        cinolib::Color warningTextColor;
        cinolib::Color backgroundColor;
        cinolib::Color overlayColor;
        cinolib::Color mutedOverlayColor;
        cinolib::Color faceColor;
        cinolib::Color edgeColor;
        cinolib::Color highlightedPolyColor;
        cinolib::Color highlightedFaceColor;
        cinolib::Color targetFaceColor;
        cinolib::Color targetEdgeColor;
        cinolib::Color dagElementColor;
        cinolib::Color dagHighlightedElementColor;
        float axesColorSat;
        float axesColorVal;

        static constexpr Theme makeLight(float _hueDeg)
        {
            const float hue{ _hueDeg / 360.0f };
            return Theme{
                .hue = hue,
                .dark = false,
                .warningTextColor{ cinolib::Color::hsv2rgb(0.1f, 1.0f, 0.75f) },
                .backgroundColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.95f) },
                .overlayColor{ cinolib::Color::hsv2rgb(hue, 0.8f, 1.0f) },
                .mutedOverlayColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.25f) },
                .faceColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.75f) },
                .edgeColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.1f) },
                .highlightedPolyColor{ cinolib::Color::hsv2rgb(hue, 0.3f, 0.85f, 0.1f) },
                .highlightedFaceColor{ cinolib::Color::hsv2rgb(hue, 0.75f, 1.0f, 0.2f) },
                .targetFaceColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.1f) },
                .targetEdgeColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.3f) },
                .dagElementColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.75f) },
                .dagHighlightedElementColor{ cinolib::Color::hsv2rgb(hue, 0.8f, 1.0f) },
                .axesColorSat = 0.9f,
                .axesColorVal = 0.8f
            };
        }

        static constexpr Theme makeDark(float _hueDeg)
        {
            const float hue{ _hueDeg / 360.0f };
            return Theme{
                .hue = hue,
                .dark = true,
                .warningTextColor{ cinolib::Color::hsv2rgb(0.2f, 0.6f, 0.6f) },
                .backgroundColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.1f) },
                .overlayColor{ cinolib::Color::hsv2rgb(hue, 0.5f, 1.0f) },
                .mutedOverlayColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.25f) },
                .faceColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.35f) },
                .edgeColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f) },
                .highlightedPolyColor{ cinolib::Color::hsv2rgb(hue, 0.75f, 0.5f, 0.1f) },
                .highlightedFaceColor{ cinolib::Color::hsv2rgb(hue, 0.75f, 1.0f, 0.2f) },
                .targetFaceColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.1f) },
                .targetEdgeColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.3f) },
                .dagElementColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.35f) },
                .dagHighlightedElementColor{ cinolib::Color::hsv2rgb(hue, 0.5f, 1.0f) },
                .axesColorSat = 0.6f,
                .axesColorVal = 1.0f
            };
        }

        static constexpr Theme make(bool _dark, float _hueDeg)
        {
            return _dark ? makeDark(_hueDeg) : makeLight(_hueDeg);
        }

        void applyImGui() const;
        void apply(Meshing::Mesher& _mesher) const;
        void apply(Widgets::Axes& _widget) const;
        void apply(Widgets::DirectVertEdit& _widget) const;
        void apply(Widgets::Target& _widget) const;
        void apply(Widgets::VertEdit& _widget) const;
        void apply(App& _app) const;
        void apply(cinolib::GLcanvas& _canvas) const;

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
        void apply(DagViewer::Widget& _widget) const;
#endif

    };

}