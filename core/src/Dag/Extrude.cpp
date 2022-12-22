#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }, m_source{ ESource::Face }, m_vertOffset{ noId }, m_faceOffsets{}, m_clockwise{ false }
	{}

	Id Extrude::vertOffset() const
	{
		return m_vertOffset;
	}

	Id& Extrude::vertOffset()
	{
		return m_vertOffset;
	}

	bool Extrude::clockwise() const
	{
		return m_clockwise;
	}

	bool& Extrude::clockwise()
	{
		return m_clockwise;
	}

	const cpputils::collections::FixedVector<Id, 3>& Extrude::faceOffsets() const
	{
		return m_faceOffsets;
	}

	cpputils::collections::FixedVector<Id, 3>& Extrude::faceOffsets()
	{
		return m_faceOffsets;
	}

	Extrude::ESource Extrude::source() const
	{
		return m_source;
	}

	Extrude::ESource& Extrude::source()
	{
		return m_source;
	}

}