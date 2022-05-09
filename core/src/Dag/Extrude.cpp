#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }, m_faceOffset{}
	{}

	Id& Extrude::faceOffset()
	{
		return m_faceOffset;
	}

	Id Extrude::faceOffset() const
	{
		return m_faceOffset;
	}

}