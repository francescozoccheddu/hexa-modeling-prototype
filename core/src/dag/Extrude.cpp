#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }
	{}

	Id& Extrude::faceOffset()
	{
		return m_offset;
	}

	Id Extrude::faceOffset() const
	{
		return m_offset;
	}

}