#pragma once

#include <HMP/Dag/Operation.hpp>

namespace HMP::Dag
{

	class Extrude : public Operation
	{

	private:

		unsigned int m_offset;

	protected:


	public:

		Extrude();

		unsigned int& offset();
		unsigned int offset() const;

	};

}