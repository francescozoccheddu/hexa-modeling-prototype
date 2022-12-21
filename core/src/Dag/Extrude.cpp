#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }, m_source{ ESource::Face }, m_firstUpFaceOffset{ noId }, m_faceOffsets{}
	{}

	Id Extrude::firstUpFaceOffset() const
	{
		return m_firstUpFaceOffset;
	}

	Id& Extrude::firstUpFaceOffset()
	{
		return m_firstUpFaceOffset;
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