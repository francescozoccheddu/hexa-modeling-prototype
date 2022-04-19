#pragma once

#include <hexa-modeling-prototype/dag/Operation.hpp>
#include <hexa-modeling-prototype/refinementschemes.hpp>
#include <array>

namespace HMP::Dag
{

	class Refine : public Operation
	{

	private:

		EScheme m_scheme;
		bool m_needsTopologyFix;
		std::array<unsigned int, 8> m_vertices;

	public:

		Refine();

		EScheme& scheme();
		EScheme scheme() const;

		std::array<unsigned int, 8>& vertices();
		const std::array<unsigned int, 8>& vertices() const;

		bool& needsTopologyFix();
		bool needsTopologyFix() const;

	};

}