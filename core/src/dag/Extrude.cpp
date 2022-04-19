#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }
	{}

	unsigned int& Extrude::offset()
	{
		return m_offset;
	}

	unsigned int Extrude::offset() const
	{
		return m_offset;
	}

}