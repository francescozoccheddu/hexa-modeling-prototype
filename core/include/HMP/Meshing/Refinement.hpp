#pragma once

#include <HMP/Meshing/types.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cstddef>
#include <vector>
#include <array>

namespace HMP::Meshing
{

	class Refinement final : public cpputils::mixins::ReferenceClass
	{

	private:

		const std::vector<PolyVerts> m_polyCoords;

	public:

		explicit Refinement(std::vector<PolyVerts>&& _effects);

		std::vector<PolyVerts> apply(const PolyVerts& _source) const;

		std::size_t polyCount() const;

	};

}

