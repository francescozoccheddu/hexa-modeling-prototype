#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Operation.hpp>

namespace HMP::Dag
{

	class Extrude final : public Operation
	{

	private:

		Id m_faceOffset;

	public:

		Extrude();

		Id& faceOffset();
		Id faceOffset() const;

	};

}