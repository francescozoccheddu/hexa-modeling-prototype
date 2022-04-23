#pragma once

#include <HMP/types.hpp>
#include <vector>
#include <array>

namespace HMP::Refinement
{

	class Scheme final // TODO Use array instead of vector and compute refinement here
	{

	public:

		template<typename T>
		using NestedVector = std::vector<std::vector<std::vector<T>>>;

		NestedVector<Id> offsets;
		NestedVector<Real> weights;

		std::vector<PolyVerts> apply(const std::vector<Vec>& _source) const;

		std::size_t polyCount() const;

	};

}