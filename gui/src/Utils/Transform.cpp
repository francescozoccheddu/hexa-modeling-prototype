#include <HMP/Gui/Utils/Transform.hpp>

#include <cinolib/gl/glcanvas.h>
#include <cmath>

namespace HMP::Gui::Utils
{

	bool Transform::isNull(Real _value, Real epsilon)
	{
		return _value >= -epsilon && _value <= epsilon;
	}

	bool Transform::isNull(const Vec& _vec, Real epsilon)
	{
		return isNull(_vec.x(), epsilon) &&
			isNull(_vec.y(), epsilon) &&
			isNull(_vec.z(), epsilon);
	}

	Real Transform::wrapAngle(Real _angleDeg)
	{
		if (_angleDeg < 0)
		{
			return 360.0f - static_cast<Real>(std::fmod(-_angleDeg, 360.0f));
		}
		else
		{
			return static_cast<Real>(std::fmod(_angleDeg, 360.0f));
		}
	}

	Vec Transform::wrapAngles(const Vec& _anglesDeg)
	{
		return {
			wrapAngle(_anglesDeg.x()),
			wrapAngle(_anglesDeg.y()),
			wrapAngle(_anglesDeg.z())
		};
	}

	Vec Transform::rotationMatToVec(const Mat4& _mat)
	{
		Real sy = std::sqrt(_mat(0, 0) * _mat(0, 0) + _mat(1, 0) * _mat(1, 0));
		const bool singular{ sy < 1e-6 };
		Vec vec;
		if (!singular)
		{
			vec.x() = std::atan2(_mat(2, 1), _mat(2, 2));
			vec.y() = std::atan2(-_mat(2, 0), sy);
			vec.z() = std::atan2(_mat(1, 0), _mat(0, 0));
		}
		else
		{
			vec.x() = std::atan2(-_mat(1, 2), _mat(1, 1));
			vec.y() = std::atan2(-_mat(2, 0), sy);
			vec.z() = 0;
		}
		return vec;
	}

	Real Transform::avgScale() const
	{
		return (scale.x() + scale.y() + scale.z()) / 3;
	}

	Mat4 Transform::matrix() const
	{
		const Mat3 rotation3{
			Mat3::ROT_3D(cinolib::GLcanvas::world_right(), cinolib::to_rad(rotation.x())) *
			Mat3::ROT_3D(cinolib::GLcanvas::world_up(), cinolib::to_rad(rotation.y())) *
			Mat3::ROT_3D(cinolib::GLcanvas::world_forward(), cinolib::to_rad(rotation.z()))
		};
		const Mat4 rotation{
			rotation3(0,0),	rotation3(0,1),	rotation3(0,2),	0,
			rotation3(1,0),	rotation3(1,1),	rotation3(1,2),	0,
			rotation3(2,0),	rotation3(2,1),	rotation3(2,2),	0,
			0,				0,				0,				1
		};
		const Mat4 scale{ Mat4::DIAG(this->scale.add_coord(1.0)) };
		const Mat4 translation{ Mat4::TRANS(this->translation + origin) };
		const Mat4 origin{ Mat4::TRANS(-this->origin) };
		return translation * rotation * scale * origin;
	}

	bool Transform::isIdentity(Real epsilon) const
	{
		return
			isNull(translation, epsilon) &&
			isNull(scale - Vec{ 1 }, epsilon) &&
			isNull(wrapAngles(rotation + Vec{ 180 }) - Vec{ 180 }, epsilon);
	}

}