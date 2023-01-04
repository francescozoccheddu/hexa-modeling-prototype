#include <HMP/Gui/Utils/Drawing.hpp>

#include <algorithm>
#include <limits>

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

    void dashedLine(ImDrawList& _drawList, const EdgeVertData<ImVec2>& _verts, ImU32 _color, float _thickness, float _spacing)
    {
        if (cull(_verts))
        {
            return;
        }
        const cinolib::vec2f from{ _verts[0].x, _verts[0].y }, to{ _verts[1].x, _verts[1].y };
        const float length{ static_cast<float>(from.dist(to)) };
        if (length <= _spacing * 2.0f || _spacing <= 0.0f)
        {
            line(_drawList, _verts, _color, _thickness);
        }
        else
        {
            const cinolib::vec2f dir{ (to - from) / length };
            float t1{}, t2{ _spacing };
            while (t1 < length)
            {
                const cinolib::vec2f v1{ from + dir * t1 }, v2{ from + dir * t2 };
                line(_drawList, { ImVec2{ v1.x(), v1.y() }, ImVec2{ v2.x(), v2.y() } }, _color, _thickness);
                t1 += _spacing * 2.0f;
                t2 += _spacing * 2.0f;
                t2 = std::min(length, t2);
            }
        }
    }

    void line(ImDrawList& _drawList, const EdgeVertData<ImVec2>& _verts, ImU32 _color, float _thickness)
    {
        if (cull(_verts))
        {
            return;
        }
        _drawList.AddLine(_verts[0], _verts[1], _color, _thickness);
    }

    void circle(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness)
    {
        if (cull(_center, _radius))
        {
            return;
        }
        _drawList.AddCircle(_center, _radius, _color, segCount(_radius), _thickness);
    }

    void circleFilled(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color)
    {
        if (cull(_center, _radius))
        {
            return;
        }
        _drawList.AddCircleFilled(_center, _radius, _color, segCount(_radius));
    }

    void cross(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness)
    {
        if (cull(_center, _radius))
        {
            return;
        }
        line(_drawList, { ImVec2{ _center.x, _center.y - _radius }, ImVec2{ _center.x, _center.y + _radius } }, _color, _thickness);
        line(_drawList, { ImVec2{ _center.x - _radius, _center.y }, ImVec2{ _center.x + _radius, _center.y } }, _color, _thickness);
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
        if (cull(topLeft, { topLeft.x + size.x, topLeft.y + size.y }))
        {
            return;
        }
        _drawList.AddText(ImGui::GetFont(), _size, topLeft, _color, _text, nullptr);
    }

    void quad(ImDrawList& _drawList, const QuadVertData<ImVec2>& _verts, ImU32 _color, float _thickness)
    {
        if (cull(_verts))
        {
            return;
        }
        _drawList.AddQuad(_verts[0], _verts[1], _verts[2], _verts[3], _color, _thickness);
    }

    void quadFilled(ImDrawList& _drawList, const QuadVertData<ImVec2>& _verts, ImU32 _color)
    {
        if (cull(_verts))
        {
            return;
        }
        _drawList.AddQuadFilled(_verts[0], _verts[1], _verts[2], _verts[3], _color);
    }

    bool cull(const ImVec2& _min, const ImVec2& _max)
    {
        const ImVec2 smin{ ImGui::GetWindowPos() }, ssize{ ImGui::GetWindowSize() }, smax{ smin.x + ssize.x, smin.y + ssize.y };
        return _min.x > smax.x
            || _min.y > smax.y
            || _max.x < smin.x
            || _max.y < smin.y;
    }

    bool cull(const ImVec2& _center, float _halfSize)
    {
        return cull({ _center.x - _halfSize, _center.y - _halfSize }, { _center.x + _halfSize, _center.y + _halfSize });
    }

    bool cull(const ImVec2* _begin, const ImVec2* _end)
    {
        ImVec2
            min{ std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() },
            max{ -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity() };
        while (_begin != _end)
        {
            const ImVec2& vert{ *_begin++ };
            if (vert.x < min.x)
            {
                min.x = vert.x;
            }
            if (vert.x > max.x)
            {
                max.x = vert.x;
            }
            if (vert.y < min.y)
            {
                min.y = vert.y;
            }
            if (vert.y > max.y)
            {
                max.y = vert.y;
            }
        }
        return cull(min, max);
    }

    bool cull(const QuadVertData<ImVec2>& _verts)
    {
        return cull(_verts.begin(), _verts.end());
    }

    bool cull(const EdgeVertData<ImVec2>& _verts)
    {
        return cull(_verts.begin(), _verts.end());
    }

}