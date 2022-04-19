#pragma once

#include <vector>
#include <memory>
#include <HMP/grid.hpp>
#include <HMP/undoredo.hpp>

namespace HMP
{

	class Grid;
	class RefineAction : public Action
	{
	public:
		RefineAction(Grid& grid, unsigned int pid);

		void execute() override;
		void undo() override;

	private:

		Grid& grid;
		unsigned int pid;
		std::vector<unsigned int> vids;
		std::shared_ptr<Operation> op;
	};

}