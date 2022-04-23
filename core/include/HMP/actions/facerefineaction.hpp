#pragma once

#include <HMP/types.hpp>
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
		FaceRefineAction(Grid& grid, Id fid);

		void apply() override;
		void unapply() override;

	private:

		Grid& grid;
		Id pid;
		Id face_off;
		std::vector<Id> vids;
		Dag::Operation* op;
	};

}