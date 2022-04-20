#include <HMP/actions/refineaction.hpp>

namespace HMP
{

	RefineAction::RefineAction(Grid& grid, unsigned int pid) : grid(grid), pid(pid) {}

	void RefineAction::execute()
	{
		auto vids = grid.mesh.poly_verts_id(pid);
		auto element = grid.mesh.poly_data(pid).element;
		auto refine = grid.op_tree.refine(element);
		this->op = refine;
		this->vids = vids;

		if (refine == nullptr) return;

		grid.refine(pid, refine, false);

		grid.refine_queue.push_back(refine);

		for (auto& child : refine->children)
		{

			unsigned int pid = child->pid;

			//update displacement
			for (unsigned int off = 0; off < 8; off++)
			{
				grid.op_tree.move(child, off, grid.mesh.poly_vert(pid, off));
			}
		}

	}

	void RefineAction::undo()
	{
		grid.op_tree.prune(this->op);
		std::vector<unsigned int> polys_to_remove;
		for (const auto& child : op->children)
		{

			unsigned int pid = child->pid;
			polys_to_remove.push_back(pid);
		}
		std::sort(polys_to_remove.begin(), polys_to_remove.end(), std::greater<unsigned int>());
		for (unsigned int pid : polys_to_remove) grid.mesh.poly_remove(pid, false);
		//unsigned int pid = grid.mesh.poly_add(this->vids);
		grid.mesh.poly_data(pid).flags[cinolib::HIDDEN] = false;

		grid.refine_queue.pop_back();
	}

}