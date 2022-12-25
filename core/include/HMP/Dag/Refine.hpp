#pragma once

#include <HMP/Dag/Operation.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Refinement/Schemes.hpp>

namespace HMP::Dag
{

	class Refine final: public Operation
	{

	private:

		I m_forwardFi, m_firstVi;
		Refinement::EScheme m_scheme;

	public:

		Refine();

		I& forwardFi();
		I forwardFi() const;

		I& firstVi();
		I firstVi() const;

		Refinement::EScheme& scheme();
		Refinement::EScheme scheme() const;

	};

}