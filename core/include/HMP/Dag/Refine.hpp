#pragma once

#include <HMP/Dag/Operation.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/refinementSchemes.hpp>

namespace HMP::Dag
{

	class Refine final : public Operation
	{

	private:

		Id m_forwardFaceOffset, m_upFaceOffset;
		Meshing::ERefinementScheme m_scheme;

	public:

		Refine();

		Id& forwardFaceOffset();
		Id forwardFaceOffset() const;

		Id& upFaceOffset();
		Id upFaceOffset() const;

		Meshing::ERefinementScheme& scheme();
		Meshing::ERefinementScheme scheme() const;

	};

}