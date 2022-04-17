#include <hexa-modeling-prototype/actions/extrudeaction.hpp>

namespace HMP
{

	ExtrudeAction::ExtrudeAction(Grid& grid, unsigned int pid, unsigned int face_offset) : grid(grid), pid(pid), face_offset(face_offset) {}

	void ExtrudeAction::execute()
	{
		auto vids = grid.mesh.poly_verts_id(pid);
		auto id = grid.mesh.poly_verts_id(pid, true);
		auto element = grid.vids2element()[id];
		auto extrude = grid.op_tree.extrude(element, face_offset);
		if (extrude == nullptr) return;

		grid.extrude(pid, face_offset, extrude);

		//update displacement
		auto& new_element = extrude->children.front();
		auto new_id = grid.element2vids()[new_element];
		this->id = new_id;
		this->op = extrude;
		unsigned int new_pid = grid.vids2pid(new_id);
		for (unsigned int off = 0; off < 8; off++)
		{
			grid.op_tree.move(new_element, off, grid.mesh.poly_vert(new_pid, off));
		}

	}

	void ExtrudeAction::undo()
	{
		grid.op_tree.prune(this->op);
		unsigned int pid = grid.vids2pid(id);
		grid.mesh.poly_remove(pid, false);
	}

}