#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <HMP/grid.hpp>
#include <HMP/undoredo.hpp>

namespace HMP
{

	class Grid;
	class MakeConformingAction : public Action
	{
	public:
		MakeConformingAction(Grid& grid);

		void execute() override;
		void undo() override;

	private:

		Grid& grid;
		std::vector<std::pair<std::vector<unsigned int>, bool>> polys_to_revert;
		std::vector<std::vector<unsigned int>> ids;
		std::vector<unsigned int> pids;
		std::deque<std::shared_ptr<Refine>> queue_to_revert;
	};

}