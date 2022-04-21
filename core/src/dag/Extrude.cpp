#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }
	{}

	unsigned int& Extrude::faceOffset()
	{
		return m_offset;
	}

	unsigned int Extrude::faceOffset() const
	{
		return m_offset;
	}

}