#pragma once

#include <vector>
#include <memory>
#include <HMP/grid.hpp>
#include <HMP/undoredo.hpp>

namespace HMP
{

	class Grid;
	class ExtrudeAction : public Action
	{
	public:
		ExtrudeAction(Grid& grid, unsigned int pid, unsigned int face_offset);

		void execute() override;
		void undo() override;

	private:

		Grid& grid;
		unsigned int pid;
		unsigned int face_offset;
		unsigned int new_pid;
		std::shared_ptr<Operation> op;
	};

}