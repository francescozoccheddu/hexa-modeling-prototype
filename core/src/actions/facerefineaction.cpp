#include <hexa-modeling-prototype/actions/facerefineaction.hpp>
#include <assert.h>

FaceRefineAction::FaceRefineAction(Grid& grid, unsigned int fid) : grid(grid)
{
	assert(grid.mesh.face_is_on_srf(fid));
	this->pid = grid.mesh.adj_f2p(fid).front();
	this->face_off = grid.mesh.poly_face_offset(pid, fid);

}

void FaceRefineAction::execute()
{
	auto& mesh = grid.mesh;

	auto vids = mesh.poly_verts_id(pid);
	auto id = mesh.poly_verts_id(pid, true);
	auto element = grid.id2element()[id];
	auto refine = grid.op_tree.refine(element, 6);

	unsigned int fid = mesh.poly_face_id(pid, face_off);

	if (refine == nullptr) return;

	this->op = refine;
	this->vids = vids;

	refine->scheme_type = FACEREF;

	std::vector<unsigned int> base_vids = mesh.face_verts_id(mesh.poly_face_opposite_to(pid, fid));
	std::vector<unsigned int> opposite_vids = mesh.face_verts_id(fid);

	if (mesh.poly_face_winding(pid, fid))
	{
		std::reverse(base_vids.begin(), base_vids.end());
	}
	if (!mesh.poly_face_winding(pid, mesh.poly_face_opposite_to(pid, fid)))
	{
		std::reverse(opposite_vids.begin(), opposite_vids.end());
	}

	while (mesh.edge_id(base_vids[0], opposite_vids[0]) == -1)
	{
		std::rotate(opposite_vids.begin(), opposite_vids.begin() + 1, opposite_vids.end());
	}


	refine->vert_map.resize(8);

	for (unsigned int i = 0; i < 4; i++)
	{
		refine->vert_map[i] = mesh.poly_vert_offset(pid, base_vids[i]);
	}
	for (unsigned int i = 4; i < 8; i++)
	{
		refine->vert_map[i] = mesh.poly_vert_offset(pid, opposite_vids[i - 4]);
	}


	grid.refine(pid, refine);

	for (auto& child : refine->children)
	{

		unsigned int pid = grid.id2pid(grid.element2id()[child]);

		//update displacement
		for (unsigned int off = 0; off < 8; off++)
		{
			grid.op_tree.move(child, off, mesh.poly_vert(pid, off));
		}
	}


	mesh.poly_remove(pid, false);

}

void FaceRefineAction::undo()
{

	grid.op_tree.prune(this->op);
	std::vector<unsigned int> polys_to_remove;
	for (const auto& child : op->children)
	{

		unsigned int pid = grid.id2pid(grid.element2id()[child]);
		polys_to_remove.push_back(pid);
		grid.id2element().erase(grid.element2id()[child]);
		grid.element2id().erase(child);

	}
	std::sort(polys_to_remove.begin(), polys_to_remove.end(), std::greater<unsigned int>());
	for (unsigned int pid : polys_to_remove) grid.mesh.poly_remove(pid, false);
	unsigned int pid = grid.mesh.poly_add(this->vids);
	this->pid = pid;
	grid.mesh.poly_data(pid).flags[cinolib::HIDDEN] = false;

}
