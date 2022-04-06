#include <hexa-modeling-prototype/grid.hpp>
#include <unordered_set> // FIXME
#include <assert.h> // FIXME
#include <cinolib/octree.h>
#include <cmath>

namespace HMP
{

	constexpr double pi = 3.14159265358979323846;
	constexpr double cubeSize = 0.5;

	Grid::Grid() : command_manager(CommandManager::get_instance())
	{
		init();
	}



	void Grid::init()
	{

		op_tree.clear();
		id2element().clear();
		std::vector<cinolib::vec3d> init_cube_coords = { cinolib::vec3d(-cubeSize,-cubeSize,-cubeSize), cinolib::vec3d(-cubeSize,-cubeSize, cubeSize), cinolib::vec3d(cubeSize,-cubeSize,cubeSize), cinolib::vec3d(cubeSize,-cubeSize,-cubeSize),
														cinolib::vec3d(-cubeSize,cubeSize,-cubeSize), cinolib::vec3d(-cubeSize,cubeSize, cubeSize), cinolib::vec3d(cubeSize,cubeSize,cubeSize), cinolib::vec3d(cubeSize,cubeSize,-cubeSize) };
		std::vector<std::vector<unsigned int>> init_cube_polys = { {0,1,2,3,4,5,6,7} };
		poly_vert_ordering(init_cube_coords, init_cube_polys[0]);

		mesh.clear();
		mesh.init(init_cube_coords, init_cube_polys);

		id2element()[mesh.poly_verts_id(0, true)] = op_tree.root;
		element2id()[op_tree.root] = mesh.poly_verts_id(0, true);

		//update displacement
		for (unsigned int off = 0; off < 8; off++)
		{
			op_tree.move(op_tree.root, off, mesh.poly_vert(0, off));
		}

		v_map.clear();
		command_manager.clear();
		refine_queue.clear();

		for (unsigned int i = 0; i < 8; i++)
		{
			v_map[init_cube_coords[i]] = i;
		}

		update_mesh();


	}

	void Grid::move_vert(unsigned int vid, const cinolib::vec3d& displacement)
	{

		add_move(vid, displacement);
	}

	void Grid::move_edge(unsigned int eid, const cinolib::vec3d& displacement)
	{
		for (unsigned int vid : mesh.adj_e2v(eid))
		{

			add_move(vid, displacement);

		}
		update_mesh();
	}

	void Grid::move_face(unsigned int fid, const cinolib::vec3d& displacement)
	{

		for (unsigned int vid : mesh.adj_f2v(fid))
		{

			add_move(vid, displacement);

		}
		update_mesh();
	}

	//ADD OPERATIONS############################################################################################################################


	void Grid::add_refine(unsigned int pid)
	{
		auto refine_action = std::shared_ptr<Action>(new RefineAction(*this, pid));
		std::list<std::shared_ptr<Action>> action_list;
		action_list.push_back(refine_action);
		command_manager.execute(action_list);
	}

	void Grid::add_face_refine(unsigned int fid)
	{

		auto face_refine_action = std::shared_ptr<Action>(new FaceRefineAction(*this, fid));
		std::list<std::shared_ptr<Action>> action_list;
		action_list.push_back(face_refine_action);
		command_manager.execute(action_list);

	}

	void Grid::add_extrude(unsigned int pid, unsigned int face_offset)
	{

		auto extrude_action = std::shared_ptr<Action>(new ExtrudeAction(*this, pid, face_offset));
		std::list<std::shared_ptr<Action>> action_list;
		action_list.push_back(extrude_action);
		command_manager.execute(action_list);

	}

	void Grid::add_move(unsigned int vid, const cinolib::vec3d& displacement)
	{

		auto move_action = std::shared_ptr<Action>(new MoveAction(*this, vid, displacement));
		std::list<std::shared_ptr<Action>> action_list;
		action_list.push_back(move_action);
		command_manager.execute(action_list);

	}

	void Grid::add_remove(unsigned int pid)
	{
		auto remove_action = std::shared_ptr<Action>(new RemoveAction(*this, pid));
		std::list<std::shared_ptr<Action>> action_list;
		action_list.push_back(remove_action);
		command_manager.execute(action_list);
	}




	//REMOVE OPERATIONS############################################################################################################################

	void Grid::undo()
	{
		command_manager.undo();
		update_mesh();

	}

	void Grid::redo()
	{
		command_manager.redo();
		update_mesh();
	}

	void Grid::prune_tree(const std::shared_ptr<Operation>& operation, bool is_user_defined)
	{

		/*    std::vector<unsigned int> polys_to_remove;
			remove_operation(operation, polys_to_remove);

			for(auto &node : operation->parents){
				auto parent = std::static_pointer_cast<Element>(node);
				parent->operations.remove(operation);

			}

			operation->parents.clear();

			std::sort(polys_to_remove.begin(), polys_to_remove.end(), std::greater());
			for(unsigned int pid : polys_to_remove) mesh.poly_remove(pid, false);

			update_mesh();*/

	}


	void Grid::apply_tree_recursive(const std::list<std::shared_ptr<Operation>>& operations, unsigned int pid, bool is_user_defined)
	{

		for (const auto& op : operations)
		{
			switch (op->primitive)
			{
				case REFINE:
				{
					auto re = std::static_pointer_cast<Refine>(op);
					if (!is_user_defined) refine_queue.push_back(re);
					refine(pid, re, true);
					break;
				}
				case EXTRUDE:
				{
					auto ex = std::static_pointer_cast<Extrude>(op);
					extrude(pid, ex->offset, ex);
					break;
				}

				case REMOVE:
					remove(pid);
					break;
			}

			for (unsigned int i = 0; i < op->children.size(); i++)
			{
				const auto& child = std::static_pointer_cast<Element>(op->children[i]);
				apply_tree_recursive(child->operations, id2pid(element2id()[child]), false);
			}

		}



	}



	void Grid::apply_tree(const std::list<std::shared_ptr<Operation>>& operations, unsigned int pid, bool is_user_defined)
	{

		apply_tree_recursive(operations, pid, is_user_defined);


		for (unsigned int pid = 0; pid < mesh.num_polys(); pid++)
		{
			for (unsigned int off = 0; off < 8; off++)
			{
				unsigned int vid = mesh.poly_vert_id(pid, off);
				const auto& el = id2element()[mesh.poly_verts_id(pid, true)];
				move(vid, el->displacements[off] - mesh.vert(vid));
			}
		}

		update_mesh();
	}

	void Grid::update_displacement_for_op(const std::shared_ptr<Operation>& op)
	{
		for (auto& child : op->children)
		{

			unsigned int pid = id2pid(element2id()[child]);

			//update displacement
			for (unsigned int off = 0; off < 8; off++)
			{
				op_tree.move(child, off, mesh.poly_vert(pid, off));
			}
		}
	}

	void Grid::make_conforming()
	{
		auto make_conforming_action = std::shared_ptr<Action>(new MakeConformingAction(*this));
		std::list<std::shared_ptr<Action>> action_list;
		action_list.push_back(make_conforming_action);
		command_manager.execute(action_list);
		update_mesh();
	}

	void Grid::project_on_target(cinolib::Trimesh<>& target)
	{
		cinolib::Quadmesh<> peel;
		std::unordered_map<unsigned int, unsigned int> h2q, q2h;
		cinolib::export_surface(mesh, peel, h2q, q2h);

		std::unordered_map<unsigned int, unsigned int> m_map;
		std::unordered_map<unsigned int, unsigned int> v_map;
		cinolib::export_surface(mesh, peel, m_map, v_map);
		target.edge_set_flag(cinolib::MARKED, false);
		peel.edge_set_flag(cinolib::MARKED, false);
		cinolib::Quadmesh tmp_peel = peel;
		target.edge_mark_sharp_creases();
		cinolib::Octree tree;
		tree.build_from_mesh_edges(peel);

		cinolib::SmootherOptions options;
		options.n_iters = 3;
		options.laplacian_mode = cinolib::UNIFORM;
		cinolib::mesh_smoother(peel, target, options);
		std::cout << "Projecting..." << std::endl;

		unsigned int count = 0;
		for (unsigned int vid = 0; vid < peel.num_verts(); vid++)
		{
			if (!(mesh.vert(v_map[vid]) == peel.vert(vid)))
			{
				auto displacement = peel.vert(vid) - mesh.vert(v_map[vid]);
				add_move(v_map[vid], displacement);
				count++;
			}
		}

		command_manager.collapse_last_n_actions(count);

		update_mesh();

		std::cout << "Projection completed" << std::endl;

	}

	void Grid::apply_transform(std::shared_ptr<Operation>& op, Transform T)
	{

		if (op->primitive != EXTRUDE && op->primitive != REFINE) return;

		switch (T)
		{
			case REFLECT_XZ:
				if (op->primitive == EXTRUDE) std::static_pointer_cast<Extrude>(op)->offset = xz_refl_mask[std::static_pointer_cast<Extrude>(op)->offset];
				break;
			case REFLECT_XY:
				if (op->primitive == EXTRUDE) std::static_pointer_cast<Extrude>(op)->offset = xy_refl_mask[std::static_pointer_cast<Extrude>(op)->offset];
				break;
			case REFLECT_YZ:
				if (op->primitive == EXTRUDE) std::static_pointer_cast<Extrude>(op)->offset = yz_refl_mask[std::static_pointer_cast<Extrude>(op)->offset];
				break;
			case ROTATE_X:
				if (op->primitive == EXTRUDE) std::static_pointer_cast<Extrude>(op)->offset = x_rot_mask[std::static_pointer_cast<Extrude>(op)->offset];
				if (op->primitive == REFINE) for (unsigned int& value : std::static_pointer_cast<Refine>(op)->vert_map) value = apply_rotation(0, value, 3);
				break;
			case ROTATE_Y:
				if (op->primitive == EXTRUDE) std::static_pointer_cast<Extrude>(op)->offset = y_rot_mask[std::static_pointer_cast<Extrude>(op)->offset];
				if (op->primitive == REFINE) for (unsigned int& value : std::static_pointer_cast<Refine>(op)->vert_map) value = apply_rotation(1, value, 3);
				break;
			case ROTATE_Z:
				if (op->primitive == EXTRUDE) std::static_pointer_cast<Extrude>(op)->offset = z_rot_mask[std::static_pointer_cast<Extrude>(op)->offset];
				if (op->primitive == REFINE) for (unsigned int& value : std::static_pointer_cast<Refine>(op)->vert_map) value = apply_rotation(2, value, 3);
				break;
			default:
				break;
		}

	}

	bool Grid::merge(unsigned int pid_source, unsigned int pid_dest, Transform T)
	{

		std::list<std::shared_ptr<Operation>> new_ops;
		auto source_el = id2element()[mesh.poly_verts_id(pid_source, true)];
		auto destination_el = id2element()[mesh.poly_verts_id(pid_dest, true)];

		cinolib::vec3d centroid_dest = mesh.poly_centroid(pid_dest);

		if (op_tree.merge(source_el, destination_el, &new_ops))
		{

			std::vector<std::shared_ptr<Operation>> branch_ops;
			for (auto& op : new_ops) op_tree.get_branch_operations(op, branch_ops);
			//The offset of the first extrude primitive must be changed to reflect the destination node local frame
			//If the first primitive is a rfine, then its vertex map must be rotated to reflect the destination node local frame
			//for(auto &op : branch_ops) apply_transform(op, ROTATE_Y);
			apply_tree_recursive(new_ops, pid_dest);

			return true;

			std::map<unsigned int, cinolib::vec3d> vert_map;
			std::vector<std::shared_ptr<Element>> old_elements;
			std::vector<std::shared_ptr<Element>> new_elements;

			op_tree.get_branch_elements(source_el, old_elements);

			for (const auto& op : new_ops) op_tree.get_branch_elements(op, new_elements);

			assert(old_elements.size() == new_elements.size());

			for (unsigned int i = 0; i < old_elements.size(); i++)
			{
				const auto& el_old = old_elements[i];
				const auto& el_new = new_elements[i];
				unsigned int pid = id2pid(element2id()[el_old]);
				unsigned int new_pid = id2pid(element2id()[el_new]);
				auto& new_el = id2element()[mesh.poly_verts_id(new_pid, true)];
				for (unsigned int off = 0; off < 8; off++)
				{
					unsigned int vid = mesh.poly_vert_id(pid, off);
					unsigned int vid_dest = mesh.poly_vert_id(new_pid, off);
					if (mesh.poly_contains_vert(pid_dest, vid_dest)) continue;
					vert_map[vid_dest] = mesh.vert(vid) - mesh.poly_centroid(pid_source);
					new_el->displacements[off] = centroid_dest + vert_map[vid_dest];
				}
			}


			for (const auto& pair : vert_map)
			{
				unsigned int vid = pair.first;
				cinolib::vec3d displ = pair.second;
				cinolib::vec3d final_displ = -mesh.vert(vid) + centroid_dest + displ;
				final_displ.rotate(cinolib::vec3d(0, 1, 0), pi / 2);
				move(vid, -mesh.vert(vid) + centroid_dest + displ);
			}


			return true;
		}
		return false;
	}


	std::map<std::vector<unsigned int>, std::shared_ptr<Element> >& Grid::id2element()
	{
		return op_tree.id2element;
	}

	std::unordered_map<std::shared_ptr<Element>, std::vector<unsigned int> >& Grid::element2id()
	{
		return op_tree.element2id;
	}

	void Grid::clear()
	{
		init();
	}

	void Grid::save_as_mesh(std::string filename)
	{
		std::vector<std::vector<unsigned int>> polys;

		for (unsigned int pid = 0; pid < mesh.num_polys(); pid++)
		{
			if (!mesh.poly_data(pid).flags[cinolib::HIDDEN])
			{
				polys.push_back(mesh.poly_verts_id(pid));
			}
		}

		cinolib::Hexmesh<> m(mesh.vector_verts(), polys);

		m.save(filename.c_str());
	}

	void Grid::extrude(unsigned int pid, unsigned int offset, const std::shared_ptr<Extrude>& extrude, bool merge_vertices)
	{
		unsigned int fid = mesh.poly_face_id(pid, offset);
		cinolib::vec3d normal = mesh.face_data(fid).normal;
		std::vector<unsigned int> poly = mesh.face_verts_id(fid);
		std::vector<cinolib::vec3d> face_verts = mesh.face_verts(fid);

		//std::cout<<normal<<std::endl;

		double edge_avg_length = 0;

		for (unsigned int eid : mesh.adj_f2e(fid)) edge_avg_length += mesh.edge_length(eid);

		edge_avg_length /= mesh.adj_f2e(fid).size();

		for (auto& vert : face_verts)
		{
			cinolib::vec3d new_vert = vert + normal * edge_avg_length;

			auto query = v_map.find(new_vert);
			unsigned int new_vid = query == v_map.end() || !merge_vertices ? mesh.vert_add(new_vert) : query->second;
			if (query == v_map.end()) v_map[new_vert] = new_vid;

			poly.push_back(new_vid);
		}
		//poly_vert_ordering(mesh.vector_verts(), poly);
		unsigned int new_pid = mesh.poly_add(poly);
		//element2id[extrude->children.front()] = incremental_id;
		auto& element = extrude->children.front();
		element2id()[element] = mesh.poly_verts_id(new_pid, true);
		id2element()[mesh.poly_verts_id(new_pid, true)] = element;



		update_mesh();
	}

	void Grid::refine(unsigned int pid, const std::shared_ptr<Refine>& refine, bool remove_father)
	{

		const auto& weights = st2subdivision[refine->scheme_type].first;
		const auto& vids = st2subdivision[refine->scheme_type].second;


		for (unsigned int i = 0; i < weights.size(); i++)
		{

			std::vector<unsigned int> poly;

			for (unsigned int j = 0; j < weights[i].size(); j++)
			{
				const auto& tmp = weights[i][j];
				cinolib::vec3d vert(0, 0, 0);
				for (unsigned int k = 0; k < 4; k++)
				{
					unsigned int off = refine->vert_map.empty() ? vids[i][j][k] : refine->vert_map[vids[i][j][k]];

					vert += tmp[k] * mesh.poly_vert(pid, off);
				}

				auto query = v_map.find(vert);
				unsigned int vid = query == v_map.end() ? mesh.vert_add(vert) : query->second;
				if (query == v_map.end()) v_map[vert] = vid;

				poly.push_back(vid);
			}


			//poly_vert_ordering(mesh.vector_verts(), poly);
			unsigned int new_pid = mesh.poly_add(poly);

			auto& element = refine->children[i];
			element2id()[element] = mesh.poly_verts_id(new_pid, true);
			id2element()[mesh.poly_verts_id(new_pid, true)] = element;



		}
		if (remove_father) mesh.poly_remove(pid, false);
		else 			  mesh.poly_data(pid).flags[cinolib::HIDDEN] = true;

		update_mesh();

	}

	void Grid::move(unsigned int vid, const cinolib::vec3d& displacement)
	{
		v_map.erase(mesh.vert(vid));
		mesh.vert(vid) += displacement;
		v_map[mesh.vert(vid)] = vid;
		update_mesh();
	}

	void Grid::remove(unsigned int pid)
	{
		//mesh.poly_data(pid).flags[cinolib::HIDDEN] = true;
		mesh.poly_remove(pid, false);
		update_mesh();
	}

	int Grid::id2pid(const std::vector<unsigned int>& id) const
	{

		for (unsigned int pid : mesh.adj_v2p(id[0]))
		{
			if (mesh.poly_verts_id(pid, true) == id) return pid;
		}

		return -1;
	}



	void Grid::update_mesh()
	{
		mesh.updateGL();
	}

}