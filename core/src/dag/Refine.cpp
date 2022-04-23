#include <HMP/Dag/Refine.hpp>
#include <stdexcept>

namespace HMP::Dag
{

	Refine::Refine()
		: Operation{ EPrimitive::Refine }
	{
		for (Id i{ 0 }; i < m_vertices.size(); i++)
		{
			m_vertices[i] = i;
		}
	}

	Refinement::EScheme& Refine::scheme()
	{
		return m_scheme;
	}

	Refinement::EScheme Refine::scheme() const
	{
		return m_scheme;
	}

	PolyIds& Refine::vertices()
	{
		return m_vertices;
	}

	const PolyIds& Refine::vertices() const
	{
		return m_vertices;
	}

	bool& Refine::needsTopologyFix()
	{
		return m_needsTopologyFix;
	}

	bool Refine::needsTopologyFix() const
	{
		return m_needsTopologyFix;
	}

}