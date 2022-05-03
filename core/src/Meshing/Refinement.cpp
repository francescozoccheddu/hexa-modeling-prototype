#include <HMP/Meshing/Refinement.hpp>

#include <utility>

namespace HMP::Meshing
{

	Refinement::Effect::Effect(std::size_t _index, Real _weight)
		: index{ _index }, weight{ _weight }
	{}

	Refinement::Refinement(Refinement::EffectList&& _effects)
		: m_effects{ std::forward<Refinement::EffectList>(_effects) }
	{}

	std::vector<PolyVerts> Refinement::apply(const std::vector<Vec>& _source) const
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

	std::size_t Refinement::polyCount() const
	{
		return m_effects.size();
	}

}