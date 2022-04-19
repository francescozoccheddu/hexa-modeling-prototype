#pragma once

#include <vector>
#include <memory>
#include <HMP/grid.hpp>
#include <HMP/undoredo.hpp>

namespace HMP
{

	class Grid;
	class FaceRefineAction : public Action
	{
	public:
		FaceRefineAction(Grid& grid, unsigned int fid);

		void execute() override;
		void undo() override;

	private:

		Grid& grid;
		unsigned int pid;
		unsigned int face_off;
		std::vector<unsigned int> vids;
		std::shared_ptr<Operation> op;
	};

}