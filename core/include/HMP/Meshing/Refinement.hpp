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

	public:

		class Effect final
		{

		public:

			Real weight;
			std::size_t index;

			Effect(std::size_t _index, Real _weight);

		};

		using EffectList = std::vector<PolyVertData<std::vector<Effect>>>;

	private:

		const EffectList m_effects;

	public:

		explicit Refinement(EffectList&& _effects);

		std::vector<PolyVerts> apply(const std::vector<Vec>& _source) const;

		std::size_t polyCount() const;

	};

}

