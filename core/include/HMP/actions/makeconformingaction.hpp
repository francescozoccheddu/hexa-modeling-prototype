#pragma once

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
		std::vector<std::pair<std::vector<unsigned int>, bool>> polys_to_revert;
		std::vector<std::vector<unsigned int>> ids;
		std::vector<unsigned int> pids;
		std::deque<Dag::Refine*> queue_to_revert;
	};

}