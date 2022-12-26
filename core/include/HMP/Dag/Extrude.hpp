#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Operation.hpp>
#include <cpputils/collections/FixedVector.hpp>

namespace HMP::Dag
{

	class Extrude final: public Operation
	{

	public:

		enum class ESource
		{
			Face, Edge, Vertex
		};

		Extrude();

		I firstVi;
		cpputils::collections::FixedVector<I, 3> fis;
		ESource source;
		bool clockwise;

	};

}