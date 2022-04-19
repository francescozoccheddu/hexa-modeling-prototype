#pragma once

#include <vector>
#include <memory>
#include <HMP/grid.hpp>
#include <HMP/undoredo.hpp>

namespace HMP
{

	class Grid;
	class RemoveAction : public Action
	{
	public:
		RemoveAction(Grid& grid, unsigned int pid);

		void execute() override;
		void undo() override;

	private:

		Grid& grid;
		unsigned int pid;
		std::vector<unsigned int> vids;
		std::shared_ptr<Operation> op;
	};

}