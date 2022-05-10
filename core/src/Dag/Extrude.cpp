#include <HMP/Dag/Extrude.hpp>

namespace HMP::Dag
{

	Extrude::Extrude()
		: Operation{ EPrimitive::Extrude }, m_forwardFaceOffset{}, m_upFaceOffset{}
	{}

	Id& Extrude::forwardFaceOffset()
	{
		return m_forwardFaceOffset;
	}

	Id Extrude::forwardFaceOffset() const
	{
		return m_forwardFaceOffset;
	}

	Id& Extrude::upFaceOffset()
	{
		return m_upFaceOffset;
	}

	Id Extrude::upFaceOffset() const
	{
		return m_upFaceOffset;
	}

}