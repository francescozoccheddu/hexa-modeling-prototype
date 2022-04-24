#pragma once

#include <HMP/types.hpp>
#include <cstddef>
#include <vector>
#include <array>

namespace HMP::Refinement
{

	class Scheme final
	{

	public:

		class Effect final
		{

		public:

			Real weight;
			std::size_t index;

			Effect(std::size_t _index, Real _weight);

		};

		using EffectList = std::vector<Poly<std::vector<Effect>>>;

	private:

		const EffectList m_effects;

	public:

		explicit Scheme(EffectList&& _effects);

		std::vector<PolyVerts> apply(const std::vector<Vec>& _source) const;

		std::size_t polyCount() const;

	};

}

