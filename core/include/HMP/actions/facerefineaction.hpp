#pragma once

#include <vector>
#include <memory>
#include <HMP/grid.hpp>
#include <HMP/Commander.hpp>

namespace HMP
{

	class Grid;
	class FaceRefineAction : public Commander::Action
	{
	public:
		FaceRefineAction(Grid& grid, unsigned int fid);

		void apply() override;
		void unapply() override;

	private:

		Grid& grid;
		unsigned int pid;
		unsigned int face_off;
		std::vector<unsigned int> vids;
		Dag::Operation* op;
	};

}