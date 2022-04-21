#include <HMP/actions/extrudeaction.hpp>

namespace HMP
{

	ExtrudeAction::ExtrudeAction(Grid& grid, unsigned int pid, unsigned int face_offset) : grid(grid), pid(pid), face_offset(face_offset) {}

	void ExtrudeAction::apply()
	{
		auto vids = grid.mesh.poly_verts_id(pid);
		auto element = grid.mesh.poly_data(pid).element;
		auto extrude = grid.op_tree.extrude(*element, face_offset);
		if (extrude == nullptr) return;

		grid.extrude(pid, face_offset, *extrude);

		//update displacement
		auto& new_element = *extrude->children().begin();
		new_pid = new_element.pid();
		this->op = extrude;
		for (unsigned int off = 0; off < 8; off++)
		{
			grid.op_tree.move(new_element, off, grid.mesh.poly_vert(new_pid, off));
		}

	}

	void ExtrudeAction::unapply()
	{
		grid.op_tree.prune(*this->op);
		grid.mesh.poly_remove(new_pid, false);
	}

}