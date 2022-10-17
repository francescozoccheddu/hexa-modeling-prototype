#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Commander.hpp>
#include <vector>
#include <utility>

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
