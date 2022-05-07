#pragma once

#include <HMP/Dag/Operation.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/refinementSchemes.hpp>

namespace HMP::Dag
{

	class Refine final : public Operation
	{

	private:

		Id m_faceOffset;
		Meshing::ERefinementScheme m_scheme;
		bool m_needsTopologyFix;

	public:

		Refine();

		Id& faceOffset();
		Id faceOffset() const;

		Meshing::ERefinementScheme& scheme();
		Meshing::ERefinementScheme scheme() const;

		bool& needsTopologyFix();
		bool needsTopologyFix() const;

	};

}