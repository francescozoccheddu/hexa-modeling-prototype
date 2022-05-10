#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Operation.hpp>

namespace HMP::Dag
{

	class Extrude final : public Operation
	{

	private:

		Id m_forwardFaceOffset, m_upFaceOffset;

	public:

		Extrude();

		Id& forwardFaceOffset();
		Id forwardFaceOffset() const;

		Id& upFaceOffset();
		Id upFaceOffset() const;

	};

}