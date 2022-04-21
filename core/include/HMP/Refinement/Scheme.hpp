#pragma once

#include <vector>

namespace HMP::Refinement
{

	class Scheme final // TODO Use array instead of vector and compute refinement here
	{

	public:

		template<typename T>
		using NestedVector = std::vector<std::vector<std::vector<T>>>;

		NestedVector<unsigned int> offsets;
		NestedVector<double> weights;

		std::size_t polyCount() const;

	};

}