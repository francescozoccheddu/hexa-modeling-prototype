#pragma once

#include <cinolib/color.h>

namespace HMP::Gui::Utils
{

    struct Theme final
    {

        float hue;
        bool dark;
        cinolib::Color sbOk;
        cinolib::Color sbWarn;
        cinolib::Color sbErr;
        cinolib::Color bg;
        cinolib::Color ovHi;
        cinolib::Color ovMut;
        cinolib::Color ovWarn;
        cinolib::Color ovErr;
        cinolib::Color ovPolyHi;
        cinolib::Color ovFaceHi;
        cinolib::Color srcFace;
        cinolib::Color srcEdge;
        cinolib::Color tgtFace;
        cinolib::Color tgtEdge;
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
        cinolib::Color dagNodeEl;
        cinolib::Color dagNodeElHi;
        cinolib::Color dagNodeRefine;
        cinolib::Color dagNodeExtrude;
        cinolib::Color dagNodeDelete;
#endif
        float ovAxesSat;
        float ovAxesVal;
        float ovPathSat;
        float ovPathVal;

        static constexpr Theme makeLight(float _hueDeg = 0.0f)
        {
            const float hue{ _hueDeg / 360.0f };
            return Theme{
                .hue = hue,
                .dark = false,
                .sbOk{ cinolib::Color::hsv2rgb(0.35f, 0.6f, 0.75f)},
                .sbWarn{ cinolib::Color::hsv2rgb(0.16f, 0.6f, 0.75f)},
                .sbErr{ cinolib::Color::hsv2rgb(0.05f, 0.6f, 0.75f)},
                .bg{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.95f) },
                .ovHi{ cinolib::Color::hsv2rgb(hue, 0.8f, 1.0f) },
                .ovMut{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.25f) },
                .ovWarn{ cinolib::Color::hsv2rgb(0.16f, 1.0f, 0.75f) },
                .ovErr{ cinolib::Color::hsv2rgb(0.05f, 1.0f, 0.75f) },
                .ovPolyHi{ cinolib::Color::hsv2rgb(hue, 0.3f, 0.85f, 0.1f) },
                .ovFaceHi{ cinolib::Color::hsv2rgb(hue, 0.75f, 1.0f, 0.2f) },
                .srcFace{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.75f) },
                .srcEdge{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.1f) },
                .tgtFace{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.1f) },
                .tgtEdge{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f, 0.3f) },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
                .dagNodeEl{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.75f) },
                .dagNodeElHi{ cinolib::Color::hsv2rgb(hue, 0.8f, 1.0f) },
                .dagNodeRefine{ cinolib::Color::hsv2rgb(0.4, 0.75f, 0.75f) },
                .dagNodeExtrude{ cinolib::Color::hsv2rgb(0.6, 0.75f, 0.75f) },
                .dagNodeDelete{ cinolib::Color::hsv2rgb(0.8, 0.75f, 0.75f) },
#endif
                .ovAxesSat = 0.9f,
                .ovAxesVal = 0.8f,
                .ovPathSat = 0.9f,
                .ovPathVal = 0.8f
            };
        }

        static constexpr Theme makeDark(float _hueDeg = 0.0f)
        {
            const float hue{ _hueDeg / 360.0f };
            return Theme{
                .hue = hue,
                .dark = true,
                .sbOk{ cinolib::Color::hsv2rgb(0.35f, 0.6f, 0.75f)},
                .sbWarn{ cinolib::Color::hsv2rgb(0.16f, 0.6f, 0.75f)},
                .sbErr{ cinolib::Color::hsv2rgb(0.05f, 0.6f, 0.75f)},
                .bg{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.1f) },
                .ovHi{ cinolib::Color::hsv2rgb(hue, 0.5f, 1.0f) },
                .ovMut{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.25f) },
                .ovWarn{ cinolib::Color::hsv2rgb(0.16f, 0.6f, 0.6f) },
                .ovErr{ cinolib::Color::hsv2rgb(0.05f, 0.6f, 0.6f) },
                .ovPolyHi{ cinolib::Color::hsv2rgb(hue, 0.75f, 0.5f, 0.1f) },
                .ovFaceHi{ cinolib::Color::hsv2rgb(hue, 0.75f, 1.0f, 0.2f) },
                .srcFace{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.35f) },
                .srcEdge{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.0f) },
                .tgtFace{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.1f) },
                .tgtEdge{ cinolib::Color::hsv2rgb(hue, 0.0f, 1.0f, 0.3f) },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
                .dagNodeEl{ cinolib::Color::hsv2rgb(hue, 0.0f, 0.35f) },
                .dagNodeElHi{ cinolib::Color::hsv2rgb(hue, 0.5f, 1.0f) },
                .dagNodeRefine{ cinolib::Color::hsv2rgb(0.4, 0.75f, 0.75f) },
                .dagNodeExtrude{ cinolib::Color::hsv2rgb(0.6, 0.75f, 0.75f) },
                .dagNodeDelete{ cinolib::Color::hsv2rgb(0.8, 0.75f, 0.75f) },
#endif
                .ovAxesSat = 0.6f,
                .ovAxesVal = 1.0f,
                .ovPathSat = 0.9f,
                .ovPathVal = 0.8f
            };
        }

        static constexpr Theme make(bool _dark = true, float _hueDeg = 0.0f)
        {
            return _dark ? makeDark(_hueDeg) : makeLight(_hueDeg);
        }

    };

}