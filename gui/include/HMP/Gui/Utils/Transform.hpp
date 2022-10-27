#pragma once

#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::Utils
{

	class Transform final
	{

	public:

		static bool isNull(Real _value, Real epsilon = 1e-6);
		static bool isNull(const Vec& _vec, Real epsilon = 1e-6);
		static Real wrapAngle(Real _angleDeg);
		static Vec wrapAngles(const Vec& _anglesDeg);

		Vec origin{};
		Vec translation{};
		Vec scale{ 1.0 };
		Vec rotation{};

		Mat4 matrix() const;

		Real avgScale() const;

		bool isIdentity(Real epsilon = 1e-6) const;

	};

}