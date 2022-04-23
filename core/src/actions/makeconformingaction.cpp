#include <HMP/actions/makeconformingaction.hpp>
#include <HMP/Refinement/schemes.hpp>
#include <cinolib/stl_container_utilities.h>

namespace HMP
{

	MakeConformingAction::MakeConformingAction(Grid& grid) : grid(grid) {}


	void MakeConformingAction::apply()
	{
		auto& mesh = grid.mesh;

		std::vector<Id> to_remove;
		std::map<Id, Dag::Refine*> ref_map;
		std::deque<Dag::Refine*> c_queue = grid.refine_queue;

		//FILL CONCAVITIES

		std::set<Dag::Element*> concavities_candidates;
		Id num_polys_start = mesh.num_polys();


		while (!c_queue.empty())
		{
			const auto& op = c_queue.front();
			c_queue.pop_front();

			if (!op->needsTopologyFix()) continue;

			const auto& father = *op->parents().begin();
			Id father_pid = father.pid();

			for (Id pid : mesh.adj_p2p(father_pid))
			{
				const auto& adj = mesh.poly_data(pid).element;
				bool is_leaf = !mesh.poly_data(pid).flags[cinolib::HIDDEN];
				if (is_leaf)
				{
					if (cinolib::CONTAINS(concavities_candidates, adj))
					{
						auto st = Refinement::EScheme::StandardRefinement;
						auto refine = grid.op_tree.refine(*adj, 27);
						if (refine == nullptr)
						{
							refine = new Dag::Refine{};
						}

						refine->scheme() = st;
						refine->userDefined() = false;
						refine->dependencies().add(*op);

						grid.refine(pid, *refine);

						c_queue.push_back(refine);
						grid.refine_queue.push_back(refine);
					}
					else
					{
						concavities_candidates.insert(adj);
					}
				}
			}
		}

		std::deque<Dag::Refine*> face_queue = grid.refine_queue;
		std::deque<Dag::Refine*> edge_queue = grid.refine_queue;

		//FIX FACE ADJACENTS

		while (!face_queue.empty())
		{

			const auto& op = face_queue.front();
			face_queue.pop_front();

			if (!op->needsTopologyFix()) continue;


			const auto& father = op->parents().first();
			Id father_pid = father.pid();

			for (Id pid : mesh.adj_p2p(father_pid))
			{

				const auto& adj = grid.mesh.poly_data(pid).element;
				bool is_leaf = !mesh.poly_data(pid).flags[cinolib::HIDDEN];
				if (/*op_tree.isLeaf(adj)*/ is_leaf)
				{

					Refinement::EScheme st = Refinement::EScheme::FaceScheme;

					Id shared = mesh.poly_shared_face(father_pid, pid);


					Id num_children = Refinement::schemes.at(st)->offsets.size();
					auto refine = grid.op_tree.refine(*adj, num_children);
					if (refine == nullptr)
					{
						refine = new Dag::Refine{};
					}

					refine->scheme() = st;
					refine->userDefined() = false;
					refine->dependencies().add(*op);

					//POLY VERTS REORDERING
					std::vector<Id> shared_vids = mesh.face_verts_id(shared);
					std::vector<Id> opposite_vids = mesh.face_verts_id(mesh.poly_face_opposite_to(pid, shared));

					if (mesh.poly_face_winding(pid, shared))
					{
						std::reverse(shared_vids.begin(), shared_vids.end());
					}
					if (!mesh.poly_face_winding(pid, mesh.poly_face_opposite_to(pid, shared)))
					{
						std::reverse(opposite_vids.begin(), opposite_vids.end());
					}

					while (mesh.edge_id(shared_vids[0], opposite_vids[0]) == -1)
					{
						std::rotate(opposite_vids.begin(), opposite_vids.begin() + 1, opposite_vids.end());
					}

					for (Id i = 0; i < 4; i++)
					{
						refine->vertices()[i] = mesh.poly_vert_offset(pid, shared_vids[i]);
					}
					for (Id i = 4; i < 8; i++)
					{
						refine->vertices()[i] = mesh.poly_vert_offset(pid, opposite_vids[i - 4]);
					}

					grid.refine(pid, *refine);

					grid.update_displacement_for_op(*refine);

					to_remove.push_back(pid);

				}


			}

			to_remove.push_back(father_pid);
		}

		while (!edge_queue.empty())
		{

			const auto& op = edge_queue.front();
			edge_queue.pop_front();

			if (!op->needsTopologyFix()) continue;


			const auto& father = op->parents().first();
			Id father_pid = father.pid();

			//FIX EDGE ADJACENTS
			for (Id eid : mesh.adj_p2e(father_pid))
			{

				for (Id pid : mesh.adj_e2p(eid))
				{

					if (pid == father_pid || mesh.poly_shared_face(father_pid, pid) != -1) continue;

					const auto& adj = mesh.poly_data(pid).element;
					bool is_leaf = !mesh.poly_data(pid).flags[cinolib::HIDDEN];

					if (is_leaf)
					{
						//TODO: get the scheme type somehow and apply scheme to tree

						Refinement::EScheme st = Refinement::EScheme::EdgeScheme;


						Id num_children = Refinement::schemes.at(st)->offsets.size();
						auto refine = grid.op_tree.refine(*adj, num_children);
						if (refine == nullptr) return;

						refine->scheme() = st;
						refine->userDefined() = false;
						refine->dependencies().add(*op);

						Id base = 0;
						for (Id fid : mesh.adj_e2f(eid))
						{
							if (mesh.poly_contains_face(pid, fid))
							{
								base = fid;
								break;
							}
						}

						std::vector<Id> shared_vids = mesh.face_verts_id(base);
						std::vector<Id> opposite_vids = mesh.face_verts_id(mesh.poly_face_opposite_to(pid, base));

						if (mesh.poly_face_winding(pid, base))
						{
							std::reverse(shared_vids.begin(), shared_vids.end());
						}
						if (!mesh.poly_face_winding(pid, mesh.poly_face_opposite_to(pid, base)))
						{
							std::reverse(opposite_vids.begin(), opposite_vids.end());
						}

						while ((shared_vids[0] + shared_vids[1]) - (mesh.edge_vert_id(eid, 0) + mesh.edge_vert_id(eid, 1)) != 0)
						{
							std::rotate(shared_vids.begin(), shared_vids.begin() + 1, shared_vids.end());
						}

						while (mesh.edge_id(shared_vids[0], opposite_vids[0]) == -1)
						{
							std::rotate(opposite_vids.begin(), opposite_vids.begin() + 1, opposite_vids.end());
						}


						for (Id i = 0; i < 4; i++)
						{
							refine->vertices()[i] = mesh.poly_vert_offset(pid, shared_vids[i]);
						}
						for (Id i = 4; i < 8; i++)
						{
							refine->vertices()[i] = mesh.poly_vert_offset(pid, opposite_vids[i - 4]);
						}


						//print(refine->rotations);

						op->dependents().add(*refine);


						grid.refine(pid, *refine, false);

						to_remove.push_back(pid);
					}

				}
			}
		}

		Id num_polys_delta = mesh.num_polys() - num_polys_start;
		for (Id i = 0; i < num_polys_delta; i++)
		{
			ids.push_back(mesh.poly_verts_id(mesh.num_polys() - i - 1, true));
			pids.push_back(mesh.num_polys() - i - 1);
		}

		std::sort(to_remove.begin(), to_remove.end(), std::greater());
		auto ip = std::unique(to_remove.begin(), to_remove.end());
		to_remove.resize(std::distance(to_remove.begin(), ip));
		for (Id pid : to_remove)
		{
			if (pid < num_polys_start) polys_to_revert.push_back(std::pair(mesh.poly_verts_id(pid), !mesh.poly_data(pid).flags[cinolib::HIDDEN]));
			mesh.poly_remove(pid, false);
		}

		for (const auto& el : grid.refine_queue) queue_to_revert.push_back(el);
		grid.refine_queue.clear();

	}

	void MakeConformingAction::unapply()
	{
		std::vector<Id> polys_to_remove(pids.size());
		std::set<Dag::Operation*> ops;
		for (Id i = 0; i < pids.size(); i++)
		{
			int pid = pids[i];
			if (pid == -1) continue;
			auto& element = grid.mesh.poly_data(pid).element;
			for (Dag::Node& node : element->parents())
			{
				auto& op = node.operation();
				ops.insert(&op);
			}
			polys_to_remove[i] = pid;
		}
		for (auto& op : ops) grid.op_tree.prune(*op);

		std::sort(polys_to_remove.begin(), polys_to_remove.end(), std::greater());
		for (Id pid : polys_to_remove)
		{
			std::cout << pid << std::endl;
			grid.mesh.poly_remove(pid, false);
		}
		for (const auto& pair : polys_to_revert)
		{
			Id new_pid = grid.mesh.poly_add(pair.first);
			grid.mesh.poly_data(new_pid).flags[cinolib::HIDDEN] = pair.second;
		}
		for (const auto& el : queue_to_revert) grid.refine_queue.push_back(el);

		queue_to_revert.clear();
		ids.clear();
		pids.clear();
		polys_to_revert.clear();
	}

}