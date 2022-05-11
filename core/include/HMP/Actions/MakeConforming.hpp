#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <vector>
#include <unordered_set>
#include <utility>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class MakeConforming final : public Commander::Action
	{

	private:

		std::vector<std::pair<Dag::NodeHandle<Dag::Refine>, Dag::Element*>> m_operations;
		bool m_prepared;

		void apply() override;
		void unapply() override;

	public:

		MakeConforming();

		std::vector<std::pair<const Dag::Refine*, const Dag::Element*>> operations() const;

	};

}
