#pragma once

#include <vector>
#include <memory>
#include <hexa-modeling-prototype/grid.hpp>
#include <hexa-modeling-prototype/undoredo.hpp>

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