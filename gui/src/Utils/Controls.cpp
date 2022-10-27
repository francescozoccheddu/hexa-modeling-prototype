#include <HMP/Gui/Utils/Controls.hpp>

#include <HMP/Gui/Utils/Transform.hpp>

namespace HMP::Gui::Utils::Controls
{

	bool dragVec(const char* _label, Vec& _vec, Real _speed, Real _min, Real _max, const char* _format)
	{
		cinolib::vec3<float> xyz{ _vec.cast<float>() };
		const bool changed{ ImGui::DragFloat3(_label, xyz.ptr(), static_cast<float>(_speed) / 1000.0f, static_cast<float>(_min), static_cast<float>(_max), _format, ImGuiSliderFlags_AlwaysClamp) };
		if (changed)
		{
			_vec = xyz.cast<Real>();
		}
		return changed;
	}

	bool dragTranslationVec(const char* _label, Vec& _translation, Real _sceneSize)
	{
		return dragVec(_label, _translation, _sceneSize);
	}

	bool dragScaleVec(const char* _label, Vec& _scale, Real _defScale)
	{
		Vec percScale{ _scale * 100.0 };
		const bool changed{ dragVec(_label, percScale, 100.0 * _defScale, 100.0 / 100.0 * _defScale, 100.0 * 100.0 * _defScale, "%.2f%%") };
		if (changed)
		{
			_scale = percScale / 100.0;
		}
		return changed;
	}

	bool dragScale(const char* _label, Real& _scale, Real _defScale)
	{
		float percScale{ static_cast<float>(_scale * 100.0) };
		float defScaleF{ static_cast<float>(_defScale) };
		const bool changed{ ImGui::DragFloat(_label, &percScale, 100.0f * defScaleF / 1000.0f, 100.0f / 100.0f * defScaleF, 100.0f * 100.0f * defScaleF, "%.2f%%", ImGuiSliderFlags_AlwaysClamp) };
		if (changed)
		{
			_scale = static_cast<Real>(percScale) / 100.0;
		}
		return changed;
	}

	bool dragRotation(const char* _label, Vec& _rotation)
	{
		Vec wrappedRotation{ Transform::wrapAngles(_rotation) };
		const bool changed{ dragVec(_label, wrappedRotation, 360.0, -360.0 * 2, 360.0 * 2, "%.1f deg") };
		if (changed)
		{
			_rotation = wrappedRotation;
		}
		return changed;
	}

	bool colorButton(const char* _label, cinolib::Color& _color)
	{
		return ImGui::ColorEdit4(_label, _color.rgba, ImGuiColorEditFlags_NoInputs);
	}

	ImVec4 toImGui(const cinolib::Color& _color)
	{
		return ImVec4{ _color.r(), _color.g(), _color.b(), _color.a() };
	}

}