#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Commander.hpp>
#include <vector>
#include <unordered_map>
#include <set>
#include <utility>

namespace HMP::Actions
{

	class MakeConforming final: public Commander::Action
	{

	private:

		std::vector<std::pair<Dag::NodeHandle<Dag::Refine>, Dag::Element*>> m_operations;
		bool m_prepared;

		void apply() override;
		void unapply() override;

		using RefinementMap = std::unordered_map<Meshing::ERefinementScheme, std::vector<Dag::Refine*>>;

		RefinementMap findStandardRefinements(const std::set<Meshing::ERefinementScheme>& _schemes);

		void installInsetAdapters(std::vector<Dag::Refine*>& _insets);
		void installSubdivide3x3Adapters(std::vector<Dag::Refine*>& _sub3x3s);

	public:

		MakeConforming();

		std::vector<std::pair<const Dag::Refine*, const Dag::Element*>> operations() const;

	};

}
