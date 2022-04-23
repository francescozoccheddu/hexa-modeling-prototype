#pragma once

#include <HMP/types.hpp>
#include <HMP/Dag/Operation.hpp>

namespace HMP::Dag
{

	class Extrude : public Operation
	{

	private:

		Id m_offset{};

	protected:


	public:

		Extrude();

		Id& faceOffset();
		Id faceOffset() const;

	};

}