#pragma once

#include <HMP/Meshing/types.hpp>
#include <vector>

namespace HMP::Utils::Geometry
{

	struct VertexComparer final
	{

		bool operator()(const Vec& _a, const Vec& _b) const;

	};

	enum class EAxis
	{
		X, Y, Z
	};

	Id rotateVid(EAxis _axis, Id _vid, int _times = 1);

	Id rotateFid(EAxis _axis, Id _fid, int _times = 1);

	Id reflectFid(EAxis _axis, Id _fid, int _times = 1);

	void sortVertices(PolyVerts& _verts);

}