#pragma once

#include <HMP/Dag/Operation.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/refinementSchemes.hpp>

namespace HMP::Dag
{

	class Refine final: public Operation
	{

	private:

		I m_forwardFi, m_firstVi;
		Meshing::ERefinementScheme m_scheme;

	public:

		Refine();

		I& forwardFi();
		I forwardFi() const;

		I& firstVi();
		I firstVi() const;

		Meshing::ERefinementScheme& scheme();
		Meshing::ERefinementScheme scheme() const;

	};

}