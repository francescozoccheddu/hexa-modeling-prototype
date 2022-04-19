#include <hexa-modeling-prototype/dag/Refine.hpp>
#include <stdexcept>

namespace HMP::Dag
{

	Refine::Refine()
		: Operation{ EPrimitive::Refine }
	{}

	EScheme& Refine::scheme()
	{
		return m_scheme;
	}

	EScheme Refine::scheme() const
	{
		return m_scheme;
	}

	std::array<unsigned int, 8>& Refine::vertices()
	{
		return m_vertices;
	}

	const std::array<unsigned int, 8>& Refine::vertices() const
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