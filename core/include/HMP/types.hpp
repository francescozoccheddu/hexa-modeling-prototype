#pragma once

#include <cinolib/geometry/vec_mat.h>
#include <cstddef>
#include <array>

namespace HMP
{

	using Real = double;
	using Vec = cinolib::vec<3, Real>;
	using Vec2 = cinolib::vec<2, Real>;

	using Id = unsigned int;

	template<typename TData>
	using Poly = std::array<TData, 8>;

	template<typename TData>
	using Face = std::array<TData, 4>;

	using PolyVerts = Poly<Vec>;
	using PolyIds = Poly<Id>;
	using FaceVerts = Face<Vec>;
	using FaceIds = Face<Id>;

}