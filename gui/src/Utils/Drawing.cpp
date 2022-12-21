#include <HMP/Gui/Utils/Drawing.hpp>

#include <algorithm>

namespace HMP::Gui::Utils::Drawing
{

    ImVec2 project(const cinolib::GLcanvas& _canvas, const Vec& _point)
    {
        Vec2 point2d;
        GLdouble z;
        _canvas.project(_point, point2d, z);
        return ImVec2{
            static_cast<float>(point2d.x() * _canvas.dpi_factor()),
            static_cast<float>(point2d.y() * _canvas.dpi_factor())
        };
    }

    int segCount(float _radius)
    {
        return std::clamp(static_cast<int>(std::round(_radius / 8)), 6, 36);
    }

    void dashedLine(ImDrawList& _drawList, const ImVec2& _from, const ImVec2& _to, ImU32 _color, float _thickness, float _spacing)
    {
        const cinolib::vec2f from{ _from.x, _from.y }, to{ _to.x, _to.y };
        const float length{ static_cast<float>(from.dist(to)) };
        if (length <= _spacing * 2.0f || _spacing <= 0.0f)
        {
            line(_drawList, _from, _to, _color, _thickness);
        }
        else
        {
            const cinolib::vec2f dir{ (to - from) / length };
            float t1{}, t2{ _spacing };
            while (t1 < length)
            {
                const cinolib::vec2f v1{ from + dir * t1 }, v2{ from + dir * t2 };
                line(_drawList, { v1.x(), v1.y() }, { v2.x(), v2.y() }, _color, _thickness);
                t1 += _spacing * 2.0f;
                t2 += _spacing * 2.0f;
                t2 = std::min(length, t2);
            }
        }
    }

    void line(ImDrawList& _drawList, const ImVec2& _from, const ImVec2& _to, ImU32 _color, float _thickness)
    {
        _drawList.AddLine(_from, _to, _color, _thickness);
    }

    void circle(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness)
    {
        _drawList.AddCircle(_center, _radius, _color, segCount(_radius), _thickness);
    }

    void circleFilled(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color)
    {
        _drawList.AddCircleFilled(_center, segCount(_radius), _color, 6);
    }

    void cross(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness)
    {
        line(_drawList, { _center.x, _center.y - _radius }, { _center.x, _center.y + _radius }, _color, _thickness);
        line(_drawList, { _center.x - _radius, _center.y }, { _center.x + _radius, _center.y }, _color, _thickness);
    }

    float align(float _position, float _size, EAlign _align)
    {
        switch (_align)
        {
            default:
            case EAlign::LeftTop:
                return _position;
            case EAlign::Middle:
                return _position - _size / 2;
            case EAlign::RightBottom:
                return _position - _size;
        }
    }

    ImVec2 align(const ImVec2& _position, const ImVec2& _size, EAlign _hAlign, EAlign _vAlign)
    {
        return ImVec2{
            align(_position.x, _size.x, _hAlign),
            align(_position.y, _size.y, _vAlign)
        };
    }

    void text(ImDrawList& _drawList, const char* _text, const ImVec2& _position, float _size, ImU32 _color, EAlign _hAlign, EAlign _vAlign)
    {
        const ImVec2 defSize{ ImGui::CalcTextSize(_text) };
        const float defSizeFactor{ _size / ImGui::GetFontSize() };
        const ImVec2 size{
            defSize.x * defSizeFactor,
            defSize.y * defSizeFactor
        };
        const ImVec2 topLeft{ align(_position, size, _hAlign, _vAlign) };
        _drawList.AddText(ImGui::GetFont(), _size, topLeft, _color, _text, nullptr);
    }

    ImU32 toU32(const cinolib::Color& _color)
    {
        return ImGui::ColorConvertFloat4ToU32(ImVec4{ _color.r(), _color.g(), _color.b(), _color.a() });
    }

}