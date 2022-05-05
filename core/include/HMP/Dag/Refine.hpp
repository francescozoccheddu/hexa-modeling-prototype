#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Meshing/refinementSchemes.hpp>

namespace HMP::Dag
{

	class Refine : public Operation
	{

	private:

		Meshing::ERefinementScheme m_scheme{ Meshing::ERefinementScheme::Subdivide3x3 };
		bool m_needsTopologyFix{ true };
		PolyVertIds m_vertices{};

	public:

		Refine();

		Meshing::ERefinementScheme& scheme();
		Meshing::ERefinementScheme scheme() const;

		PolyVertIds& vertices();
		const PolyVertIds& vertices() const;

		bool& needsTopologyFix();
		bool needsTopologyFix() const;

	};

}