#include <HMP/Gui/Utils/Theme.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <cinolib/color.h>
#include <cmath>

namespace HMP::Gui::Utils
{

    inline constexpr cinolib::Color hsv(float _r, float _g, float _b, float _a = 1.0f)
    {
        return cinolib::Color::hsv2rgb(_r, _g, _b, _a);
    }

    using Drawing::toImU32;
    using Controls::toImVec4;

    static constexpr float okHue{ 0.35f }, warnHue{ 0.16f }, errHue{ 0.05f };

#ifdef HMP_GUI_ENABLE_DAG_VIEWER
    static constexpr float dagNodeRefineHueShift{ 0.4f }, dagNodeExtrudeHueShift{ 0.6f }, dagNodeDeleteHueShift{ 0.8f };
#endif

    Theme Theme::makeLight(float _hueDeg)
    {
        const float hue{ _hueDeg / 360.0f };
        return Theme{
            .hue = hue,
            .dark = false,
            .sbOk{ toImVec4(hsv(okHue, 0.6f, 0.75f)) },
            .sbWarn{ toImVec4(hsv(warnHue, 0.6f, 0.75f))},
            .sbErr{ toImVec4(hsv(errHue, 0.6f, 0.75f))},
            .bg{ hsv(hue, 0.0f, 0.95f) },
            .ovHi{ toImU32(hsv(hue, 0.8f, 1.0f)) },
            .ovMut{ toImU32(hsv(hue, 0.0f, 0.0f, 0.25f)) },
            .ovWarn{ toImU32(hsv(warnHue, 1.0f, 0.75f)) },
            .ovErr{ toImU32(hsv(errHue, 1.0f, 0.75f)) },
            .ovPolyHi{ toImU32(hsv(hue, 0.3f, 0.85f, 0.1f)) },
            .ovFaceHi{ toImU32(hsv(hue, 0.75f, 1.0f, 0.2f)) },
            .srcFace{ hsv(hue, 0.0f, 0.75f) },
            .srcEdge{ hsv(hue, 0.0f, 0.1f) },
            .tgtFace{ hsv(hue, 0.0f, 0.0f, 0.1f) },
            .tgtEdge{ hsv(hue, 0.0f, 0.0f, 0.3f) },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
            .dagNodeEl{ toImU32(hsv(hue, 0.0f, 0.6f)) },
            .dagNodeElHi{ toImU32(hsv(hue, 0.8f, 1.0f)) },
            .dagNodeElMut{ toImU32(hsv(hue, 0.0f, 0.85f)) },
            .dagNodeRefine{ toImU32(hsv(std::fmod(hue + dagNodeRefineHueShift, 1.0f), 0.75f, 0.75f)) },
            .dagNodeExtrude{ toImU32(hsv(std::fmod(hue + dagNodeExtrudeHueShift, 1.0f), 0.75f, 0.75f)) },
            .dagNodeDelete{ toImU32(hsv(std::fmod(hue + dagNodeDeleteHueShift, 1.0f), 0.75f, 0.75f)) },
#endif
            .ovAxesSat = 0.9f,
            .ovAxesVal = 0.8f,
            .ovPathSat = 0.9f,
            .ovPathVal = 0.8f
        };
    }

    Theme Theme::makeDark(float _hueDeg)
    {
        const float hue{ _hueDeg / 360.0f };
        return Theme{
            .hue = hue,
            .dark = true,
            .sbOk{ toImVec4(hsv(okHue, 0.6f, 0.75f))},
            .sbWarn{ toImVec4(hsv(warnHue, 0.6f, 0.75f))},
            .sbErr{ toImVec4(hsv(errHue, 0.6f, 0.75f))},
            .bg{ hsv(hue, 0.0f, 0.1f) },
            .ovHi{ toImU32(hsv(hue, 0.5f, 1.0f)) },
            .ovMut{ toImU32(hsv(hue, 0.0f, 1.0f, 0.25f)) },
            .ovWarn{ toImU32(hsv(warnHue, 0.6f, 0.6f)) },
            .ovErr{ toImU32(hsv(errHue, 0.6f, 0.6f)) },
            .ovPolyHi{ toImU32(hsv(hue, 0.75f, 0.5f, 0.1f)) },
            .ovFaceHi{ toImU32(hsv(hue, 0.75f, 1.0f, 0.2f)) },
            .srcFace{ hsv(hue, 0.0f, 0.35f) },
            .srcEdge{ hsv(hue, 0.0f, 0.0f) },
            .tgtFace{ hsv(hue, 0.0f, 1.0f, 0.1f) },
            .tgtEdge{ hsv(hue, 0.0f, 1.0f, 0.3f) },
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
            .dagNodeEl{ toImU32(hsv(hue, 0.0f, 0.35f)) },
            .dagNodeElHi{ toImU32(hsv(hue, 0.5f, 1.0f)) },
            .dagNodeElMut{ toImU32(hsv(hue, 0.0f, 0.2f)) },
            .dagNodeRefine{ toImU32(hsv(std::fmod(hue + dagNodeRefineHueShift, 1.0f), 0.75f, 0.75f)) },
            .dagNodeExtrude{ toImU32(hsv(std::fmod(hue + dagNodeExtrudeHueShift, 1.0f), 0.75f, 0.75f)) },
            .dagNodeDelete{ toImU32(hsv(std::fmod(hue + dagNodeDeleteHueShift, 1.0f), 0.75f, 0.75f)) },
#endif
            .ovAxesSat = 0.6f,
            .ovAxesVal = 1.0f,
            .ovPathSat = 0.9f,
            .ovPathVal = 0.8f
        };
    }

    Theme Theme::make(bool _dark, float _hueDeg)
    {
        return _dark ? makeDark(_hueDeg) : makeLight(_hueDeg);
    }

}