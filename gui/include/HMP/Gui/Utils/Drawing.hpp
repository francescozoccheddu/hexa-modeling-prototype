#pragma once

#include <imgui.h>
#include <cinolib/gl/glcanvas.h>
#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::Utils::Drawing
{

    enum class EAlign
    {
        LeftTop, Middle, RightBottom
    };

    ImVec2 project(const cinolib::GLcanvas& _canvas, const Vec& _point);

    ImU32 toU32(const cinolib::Color& _color);

    void dashedLine(ImDrawList& _drawList, const ImVec2& _from, const ImVec2& _to, ImU32 _color, float _thickness = 1.0f, float _spacing = 10.0f);

    void line(ImDrawList& _drawList, const ImVec2& _from, const ImVec2& _to, ImU32 _color, float _thickness = 1.0f);

    void circle(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness = 1.0f);

    void circleFilled(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color);

    void cross(ImDrawList& _drawList, const ImVec2& _center, float _radius, ImU32 _color, float _thickness = 1.0f);

    void text(ImDrawList& _drawList, const char* _text, const ImVec2& _position, float _size, ImU32 _color, EAlign _hAlign = EAlign::Middle, EAlign _vAlign = EAlign::Middle);

}