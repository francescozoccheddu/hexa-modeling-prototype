#pragma once

#include <vector>
#include <memory>
#include <cinolib/geometry/vec_mat.h>
#include <HMP/grid.hpp>
#include <HMP/Commander.hpp>

namespace HMP
{

	class Grid;
	class MoveAction : public Commander::Action
	{
	public:
		MoveAction(Grid& grid, unsigned int vid, cinolib::vec3d displacement);

		void apply() override;
		void unapply() override;

	private:

		Grid& grid;
		unsigned int vid;
		cinolib::vec3d displacement;
		std::vector<Dag::Element*> elements;
		std::vector<unsigned int> offsets;
	};

}