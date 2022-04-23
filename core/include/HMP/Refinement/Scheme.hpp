#pragma once

#include <vector>
#include <array>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Refinement
{

	class Scheme final // TODO Use array instead of vector and compute refinement here
	{

	public:

		template<typename T>
		using NestedVector = std::vector<std::vector<std::vector<T>>>;

		NestedVector<unsigned int> offsets;
		NestedVector<double> weights;

		std::vector<std::array<cinolib::vec3d, 8>> apply(const std::vector<cinolib::vec3d>& _source) const;

		std::size_t polyCount() const;

	};

}