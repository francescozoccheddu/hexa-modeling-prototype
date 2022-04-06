#include <hexa-modeling-prototype/actions/removeaction.hpp>

namespace HMP
{

	RemoveAction::RemoveAction(Grid& grid, unsigned int pid) : grid(grid), pid(pid) {}

	void RemoveAction::execute()
	{
		auto vids = grid.mesh.poly_verts_id(pid);
		auto id = grid.mesh.poly_verts_id(pid, true);
		this->vids = grid.mesh.poly_verts_id(pid, false);;
		auto element = grid.id2element()[id];
		auto remove = grid.op_tree.remove(element);
		this->op = remove;
		if (remove == nullptr) return;

		grid.remove(pid);

	}

	void RemoveAction::undo()
	{
		grid.op_tree.prune(this->op);
		grid.mesh.poly_add(this->vids);
	}

}