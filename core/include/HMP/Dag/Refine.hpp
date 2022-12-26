#pragma once

#include <HMP/Dag/Operation.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Refinement/Schemes.hpp>

namespace HMP::Dag
{

	class Refine final: public Operation
	{

	public:

		Refine();

		Refinement::EScheme scheme;
		I forwardFi;
		I firstVi;

	};

}