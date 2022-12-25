#include <HMP/Dag/Refine.hpp>

namespace HMP::Dag
{

	Refine::Refine()
		: Operation{ EPrimitive::Refine }, m_forwardFi{}, m_firstVi{}, m_scheme{ Refinement::EScheme::Subdivide3x3 }
	{}

	I& Refine::forwardFi()
	{
		return m_forwardFi;
	}

	I Refine::forwardFi() const
	{
		return m_forwardFi;
	}

	I& Refine::firstVi()
	{
		return m_firstVi;
	}

	I Refine::firstVi() const
	{
		return m_firstVi;
	}

	Refinement::EScheme& Refine::scheme()
	{
		return m_scheme;
	}

	Refinement::EScheme Refine::scheme() const
	{
		return m_scheme;
	}

}