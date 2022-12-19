#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }, m_source{ ESource::Face }, m_faceOffsets{ }
	{}

	Id Extrude::forwardFaceOffset() const
	{
		return m_faceOffsets[0];
	}

	Id Extrude::upFaceOffset() const
	{
		return m_faceOffsets[1];
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