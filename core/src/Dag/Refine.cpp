#include <HMP/Dag/Refine.hpp>

namespace HMP::Dag
{

	Refine::Refine()
		: Operation{ EPrimitive::Refine }, m_forwardFaceOffset{}, m_upFaceOffset{}, m_scheme{ Meshing::ERefinementScheme::Subdivide3x3 }
	{}

	Id& Refine::forwardFaceOffset()
	{
		return m_forwardFaceOffset;
	}

	Id Refine::forwardFaceOffset() const
	{
		return m_forwardFaceOffset;
	}

	Id& Refine::upFaceOffset()
	{
		return m_upFaceOffset;
	}

	Id Refine::upFaceOffset() const
	{
		return m_upFaceOffset;
	}

	Meshing::ERefinementScheme& Refine::scheme()
	{
		return m_scheme;
	}

	Meshing::ERefinementScheme Refine::scheme() const
	{
		return m_scheme;
	}

}