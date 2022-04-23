#pragma once

#include <HMP/types.hpp>
#include <memory>
#include <vector>
#include <deque>
#include <HMP/grid.hpp>
#include <HMP/Commander.hpp>

namespace HMP
{

	class Grid;
	class MakeConformingAction : public Commander::Action
	{
	public:
		MakeConformingAction(Grid& grid);

		void apply() override;
		void unapply() override;

	private:

		Grid& grid;
		std::vector<std::pair<std::vector<Id>, bool>> polys_to_revert;
		std::vector<std::vector<Id>> ids;
		std::vector<Id> pids;
		std::deque<Dag::Refine*> queue_to_revert;
	};

}