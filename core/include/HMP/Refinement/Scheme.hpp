#pragma once

#include <vector>

namespace HMP::Refinement
{

	struct Scheme final
	{

		template<typename T>
		using NestedVector = std::vector<std::vector<std::vector<T>>>;

		NestedVector<unsigned int> vertices;
		NestedVector<double> weights;

	};

}