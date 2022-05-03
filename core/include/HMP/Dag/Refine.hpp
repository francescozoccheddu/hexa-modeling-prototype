#pragma once

#include <HMP/types.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Meshing/refinementSchemes.hpp>

namespace HMP::Dag
{

	class Refine : public Operation
	{

	private:

		Meshing::ERefinementScheme m_scheme{ Meshing::ERefinementScheme::Subdivide3x3 };
		bool m_needsTopologyFix{ true };
		PolyIds m_vertices{};

	public:

		Refine();

		Meshing::ERefinementScheme& scheme();
		Meshing::ERefinementScheme scheme() const;

		PolyIds& vertices();
		const PolyIds& vertices() const;

		bool& needsTopologyFix();
		bool needsTopologyFix() const;

	};

}