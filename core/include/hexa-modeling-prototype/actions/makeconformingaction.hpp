#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <hexa-modeling-prototype/grid.hpp>
#include <hexa-modeling-prototype/undoredo.hpp>

class Grid;
class MakeConformingAction : public Action
{
public:
	MakeConformingAction(Grid& grid);

	void execute() override;
	void undo() override;

private:

	Grid& grid;
	std::vector<std::pair<std::vector<unsigned int>, bool>> polys_to_revert;
	std::vector<std::vector<unsigned int>> ids;
	std::deque<std::shared_ptr<Refine>> queue_to_revert;
};
