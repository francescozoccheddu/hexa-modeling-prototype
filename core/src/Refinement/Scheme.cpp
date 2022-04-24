#include <HMP/Refinement/Scheme.hpp>

#include <utility>

namespace HMP::Refinement
{

	Scheme::Effect::Effect(std::size_t _index, Real _weight)
		: index{ _index }, weight{ _weight }
	{}

	Scheme::Scheme(Scheme::EffectList&& _effects)
		: m_effects{ std::forward<Scheme::EffectList>(_effects) }
	{}

	std::vector<PolyVerts> Scheme::apply(const std::vector<Vec>& _source) const
	{
		std::vector<PolyVerts> polys{};
		polys.reserve(polyCount());
		for (std::size_t p{ 0 }; p < polyCount(); p++)
		{
			PolyVerts verts;
			verts.fill(Vec{ 0,0,0 });
			for (std::size_t v{ 0 }; v < 8; v++)
			{
				for (const Effect& effect : m_effects[p][v])
				{
					verts[v] += effect.weight * _source[effect.index];
				}
			}
			polys.push_back(verts);
		}
		return polys;
	}

	std::size_t Scheme::polyCount() const
	{
		return m_effects.size();
	}

}