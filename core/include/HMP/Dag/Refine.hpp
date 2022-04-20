#pragma once

#include <HMP/Dag/Operation.hpp>
#include <HMP/Refinement/schemes.hpp>
#include <array>

namespace HMP::Dag
{

	class Refine : public Operation
	{

	private:

		Refinement::EScheme m_scheme{ Refinement::EScheme::StandardRefinement };
		bool m_needsTopologyFix{ false };
		std::array<unsigned int, 8> m_vertices{};

	public:

		Refine();

		Refinement::EScheme& scheme();
		Refinement::EScheme scheme() const;

		std::array<unsigned int, 8>& vertices();
		const std::array<unsigned int, 8>& vertices() const;

		bool& needsTopologyFix();
		bool needsTopologyFix() const;

	};

}