#include <HMP/Dag/Refine.hpp>

namespace HMP::Dag
{

	Refine::Refine()
		: Operation{ EPrimitive::Refine }, m_faceOffset{}, m_scheme{ Meshing::ERefinementScheme::Subdivide3x3 }
	{}

	Id& Refine::faceOffset()
	{
		return m_faceOffset;
	}

	Id Refine::faceOffset() const
	{
		return m_faceOffset;
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