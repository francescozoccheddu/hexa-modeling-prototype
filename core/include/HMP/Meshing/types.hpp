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
	constexpr Id noId{ static_cast<Id>(-1) };

	template<typename TData>
	using PolyFaceData = std::array<TData, 6>;

	template<typename TData>
	using PolyEdgeData = std::array<TData, 12>;

	template<typename TData>
	using PolyVertData = std::array<TData, 8>;

	template<typename TData>
	using FaceVertData = std::array<TData, 4>;

	template<typename TData>
	using FaceEdgeData = std::array<TData, 4>;

	using PolyVerts = PolyVertData<Vec>;
	using PolyVertIds = PolyVertData<Id>;
	using PolyFaceIds = PolyFaceData<Id>;
	using PolyEdgeIds = PolyEdgeData<Id>;
	using FaceVerts = FaceVertData<Vec>;
	using FaceVertIds = FaceVertData<Id>;
	using FaceEdgeIds = FaceEdgeData<Id>;

}