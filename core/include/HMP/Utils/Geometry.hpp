#pragma once

#include <cinolib/geometry/vec_mat.h>
#include <vector>
#include <array>

namespace HMP::Utils::Geometry
{

	struct VertexComparer final
	{

		bool operator()(const cinolib::vec3d& _a, const cinolib::vec3d& _b) const;

	};

	enum class EAxis
	{
		X, Y, Z
	};

	unsigned int rotateVid(EAxis _axis, unsigned int _vid, int _times = 1);

	unsigned int rotateFid(EAxis _axis, unsigned int _fid, int _times = 1);

	unsigned int reflectFid(EAxis _axis, unsigned int _fid, int _times = 1);

	void sortVids(std::array<unsigned int, 8>& _vids, const std::array<cinolib::vec3d, 8>& _vertices);

}