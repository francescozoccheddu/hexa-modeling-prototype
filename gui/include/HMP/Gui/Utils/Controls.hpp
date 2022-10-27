#pragma once 

#include <HMP/Meshing/types.hpp>
#include <imgui.h>
#include <cinolib/color.h>
#include <limits>

namespace HMP::Gui::Utils::Controls
{

	bool dragVec(const char* _label, Vec& _vec, Real _speed, Real _min = -std::numeric_limits<Real>::infinity(), Real _max = std::numeric_limits<Real>::infinity(), const char* _format = "%.3f");

	bool dragTranslationVec(const char* _label, Vec& _translation, Real _sceneSize);

	bool dragScaleVec(const char* _label, Vec& _scale, Real _defScale = 1.0);

	bool dragScale(const char* _label, Real& _scale, Real _defScale = 1.0);

	bool dragRotation(const char* _label, Vec& _rotation);

	bool colorButton(const char* _label, cinolib::Color& _color);

	ImVec4 toImGui(const cinolib::Color& _color);

}