#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }, m_source{ ESource::Face }, m_faceOffsets{ noId, noId, noId }
	{}

	Id& Extrude::forwardFaceOffset()
	{
		return m_faceOffsets[0];
	}

	Id Extrude::forwardFaceOffset() const
	{
		return m_faceOffsets[0];
	}

	Id& Extrude::upFaceOffset()
	{
		return m_faceOffsets[1];
	}

	Id Extrude::upFaceOffset() const
	{
		return m_faceOffsets[1];
	}

	Id& Extrude::rightFaceOffset()
	{
		return m_faceOffsets[2];
	}

	Id Extrude::rightFaceOffset() const
	{
		return m_faceOffsets[2];
	}

	const std::array<Id, 3>& Extrude::faceOffsets() const
	{
		return m_faceOffsets;
	}

	std::array<Id, 3>& Extrude::faceOffsets()
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