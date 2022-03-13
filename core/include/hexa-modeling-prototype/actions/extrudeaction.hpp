#pragma once

#include <vector>
#include <memory>
#include <hexa-modeling-prototype/grid.hpp>
#include <hexa-modeling-prototype/undoredo.hpp>

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
	std::vector<unsigned int> id;
	std::shared_ptr<Operation> op;
};