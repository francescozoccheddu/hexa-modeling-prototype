#pragma once

#include <vector>
#include <memory>
#include <HMP/grid.hpp>
#include <HMP/Commander.hpp>

namespace HMP
{

	class Grid;
	class ExtrudeAction : public Commander::Action
	{
	public:
		ExtrudeAction(Grid& grid, unsigned int pid, unsigned int face_offset);

		void apply() override;
		void unapply() override;

	private:

		Grid& grid;
		unsigned int pid;
		unsigned int face_offset;
		unsigned int new_pid;
		Dag::Operation* op;
	};

}