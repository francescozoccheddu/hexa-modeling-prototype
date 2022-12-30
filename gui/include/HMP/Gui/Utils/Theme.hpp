#pragma once

#include <cinolib/color.h>

namespace HMP::Gui::Utils
{

    struct Theme final
    {

        float hue;
        bool dark;
        cinolib::Color warningTextColor;
        cinolib::Color commanderAppliedActionColor;
        cinolib::Color commanderUnappliedActionColor;
        cinolib::Color commanderPendingActionColor;
        cinolib::Color backgroundColor;
        cinolib::Color overlayColor;
        cinolib::Color mutedOverlayColor;
        cinolib::Color sourceFaceColor;
        cinolib::Color sourceEdgeColor;
        cinolib::Color sourceHighlightedPolyColor;
        cinolib::Color sourceHighlightedFaceColor;
        cinolib::Color targetFaceColor;
        cinolib::Color targetEdgeColor;
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
        cinolib::Color dagElementColor;
        cinolib::Color dagHighlightedElementColor;
        cinolib::Color dagRefineNodeColor;
        cinolib::Color dagExtrudeNodeColor;
        cinolib::Color dagDeleteNodeColor;
#endif
        float axesColorSat;
        float axesColorVal;
        float projectionPathColorSat;
        float projectionPathColorVal;

        static constexpr Theme makeLight(float _hueDeg = 0.0f)
        {
            const float hue{ _hueDeg / 360.0f };
            return Theme{
                .hue = hue,
                .dark = false,
                .warningTextColor{ cinolib::Color::hsv2rgb(0.1f, 1.0f, 0.75f) },
                .commanderAppliedActionColor{ cinolib::Color::hsv2rgb(0.35f, 0.75f, 0.75f)},
                .commanderUnappliedActionColor{ cinolib::Color::hsv2rgb(0.0f, 0.75f, 0.75f)},
                .commanderPendingActionColor{ cinolib::Color::hsv2rgb(0.1f, 0.75f, 0.75f)},
                .backgroundColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.95f) },
                .overlayColor{ cinolib::Color::hsv2rgb(hue, 0.8f, 1.0f) },
                .mutedOverlayColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.25f) },
                .sourceFaceColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.75f) },
                .sourceEdgeColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.1f) },
                .sourceHighlightedPolyColor{ cinolib::Color::hsv2rgb(hue, 0.3f, 0.85f, 0.1f) },
                .sourceHighlightedFaceColor{ cinolib::Color::hsv2rgb(hue, 0.75f, 1.0f, 0.2f) },
                .targetFaceColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.1f) },
                .targetEdgeColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.3f) },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
                .dagElementColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.75f) },
                .dagHighlightedElementColor{ cinolib::Color::hsv2rgb(hue, 0.8f, 1.0f) },
                .dagRefineNodeColor{ cinolib::Color::hsv2rgb(0.4, 0.75f, 0.75f) },
                .dagExtrudeNodeColor{ cinolib::Color::hsv2rgb(0.6, 0.75f, 0.75f) },
                .dagDeleteNodeColor{ cinolib::Color::hsv2rgb(0.8, 0.75f, 0.75f) },
#endif
                .axesColorSat = 0.9f,
                .axesColorVal = 0.8f,
                .projectionPathColorSat = 0.9f,
                .projectionPathColorVal = 0.8f
            };
        }

        static constexpr Theme makeDark(float _hueDeg = 0.0f)
        {
            const float hue{ _hueDeg / 360.0f };
            return Theme{
                .hue = hue,
                .dark = true,
                .warningTextColor{ cinolib::Color::hsv2rgb(0.2f, 0.6f, 0.6f) },
                .commanderAppliedActionColor{ cinolib::Color::hsv2rgb(0.35f, 0.75f, 0.75f)},
                .commanderUnappliedActionColor{ cinolib::Color::hsv2rgb(0.0f, 0.75f, 0.75f)},
                .commanderPendingActionColor{ cinolib::Color::hsv2rgb(0.1f, 0.75f, 0.75f)},
                .backgroundColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.1f) },
                .overlayColor{ cinolib::Color::hsv2rgb(hue, 0.5f, 1.0f) },
                .mutedOverlayColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.25f) },
                .sourceFaceColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.35f) },
                .sourceEdgeColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f) },
                .sourceHighlightedPolyColor{ cinolib::Color::hsv2rgb(hue, 0.75f, 0.5f, 0.1f) },
                .sourceHighlightedFaceColor{ cinolib::Color::hsv2rgb(hue, 0.75f, 1.0f, 0.2f) },
                .targetFaceColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.1f) },
                .targetEdgeColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.3f) },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
                .dagElementColor{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.35f) },
                .dagHighlightedElementColor{ cinolib::Color::hsv2rgb(hue, 0.5f, 1.0f) },
                .dagRefineNodeColor{ cinolib::Color::hsv2rgb(0.4, 0.75f, 0.75f) },
                .dagExtrudeNodeColor{ cinolib::Color::hsv2rgb(0.6, 0.75f, 0.75f) },
                .dagDeleteNodeColor{ cinolib::Color::hsv2rgb(0.8, 0.75f, 0.75f) },
#endif
                .axesColorSat = 0.6f,
                .axesColorVal = 1.0f,
                .projectionPathColorSat = 0.9f,
                .projectionPathColorVal = 0.8f
            };
        }

        static constexpr Theme make(bool _dark = true, float _hueDeg = 0.0f)
        {
            return _dark ? makeDark(_hueDeg) : makeLight(_hueDeg);
        }

    };

}