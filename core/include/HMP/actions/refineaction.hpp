#pragma once

#include <vector>
#include <memory>
#include <HMP/grid.hpp>
#include <HMP/Commander.hpp>

namespace HMP
{

	class Grid;
	class RefineAction : public Commander::Action
	{
	public:
		RefineAction(Grid& grid, unsigned int pid);

		void apply() override;
		void unapply() override;

	private:

		Grid& grid;
		unsigned int pid;
		std::vector<unsigned int> vids;
		Dag::Operation* op;
	};

}