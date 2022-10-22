#include <HMP/Meshing/Refinement.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <utility>
#include <cinolib/geometry/lerp.hpp>

namespace HMP::Meshing
{

	Refinement::Refinement(std::vector<PolyVerts>&& _polyCoords) : m_polyCoords{std::move(_polyCoords)} 
	{}

	std::vector<PolyVerts> Refinement::apply(const PolyVerts& _source) const
	{
		static constexpr PolyVertData<std::size_t> cinolibInds{ 0,1,3,2,4,5,7,6 };
		PolyVerts source;
		for (std::size_t i{}; i < 8; i++)
		{
			source[cinolibInds[i]] = _source[i];
		}
		std::vector<PolyVerts> childrenVerts;
		childrenVerts.reserve(m_polyCoords.size());
		for (const PolyVerts& coords : m_polyCoords)
		{
			PolyVerts childVerts;
			for (std::size_t i{}; i < 8; i++)
			{
				childVerts[i] = cinolib::lerp3(source, coords[cinolibInds[i]]);
			}
			childrenVerts.push_back(childVerts);
		}
		return childrenVerts;
	}

	std::size_t Refinement::polyCount() const
	{
		return m_polyCoords.size();
	}

}