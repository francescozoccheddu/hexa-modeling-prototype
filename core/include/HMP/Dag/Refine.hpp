#pragma once

#include <HMP/types.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Refinement/schemes.hpp>

namespace HMP::Dag
{

	class Refine : public Operation
	{

	private:

		Refinement::EScheme m_scheme{ Refinement::EScheme::StandardRefinement };
		bool m_needsTopologyFix{ true };
		PolyIds m_vertices{};

	public:

		Refine();

		Refinement::EScheme& scheme();
		Refinement::EScheme scheme() const;

		PolyIds& vertices();
		const PolyIds& vertices() const;

		bool& needsTopologyFix();
		bool needsTopologyFix() const;

	};

}