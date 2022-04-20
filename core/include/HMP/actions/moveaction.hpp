#pragma once

#include <vector>
#include <memory>
#include <cinolib/geometry/vec_mat.h>
#include <HMP/grid.hpp>
#include <HMP/undoredo.hpp>

namespace HMP
{

	class Grid;
	class MoveAction : public Action
	{
	public:
		MoveAction(Grid& grid, unsigned int vid, cinolib::vec3d displacement);

		void execute() override;
		void undo() override;

	private:

		Grid& grid;
		unsigned int vid;
		cinolib::vec3d displacement;
		std::vector<Dag::Element*> elements;
		std::vector<unsigned int> offsets;
	};

}