#pragma once

#include <vector>
#include <memory>
#include <cinolib/geometry/vec_mat.h>
#include <hexa-modeling-prototype/grid.hpp>
#include <hexa-modeling-prototype/undoredo.hpp>

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
		std::vector<std::shared_ptr<Element>> elements;
		std::vector<unsigned int> offsets;
	};

}