#include <HMP/grid.hpp>
#include <unordered_set> // FIXME
#include <assert.h> // FIXME
#include <cinolib/octree.h>
#include <cmath>
#include <array>
#include <HMP/Refinement/schemes.hpp>
#include <HMP/Utils/Geometry.hpp>
#include <HMP/Utils/Collections.hpp>
#include <HMP/Dag/Utils.hpp>

namespace HMP
{

	constexpr Real pi = 3.14159265358979323846;
	constexpr Real cubeSize = 0.5;
	constexpr Real c_maxVertDistance{ 1e-6 };

	Grid::Grid()
	{
		init();
	}

	Commander& Grid::commander()
	{
		return m_commander;
	}


	const Commander& Grid::commander() const
	{
		return m_commander;
	}

	void Grid::init()
	{

		op_tree.clear();
		PolyVerts init_cube_coords = { Vec(-cubeSize,-cubeSize,-cubeSize), Vec(-cubeSize,-cubeSize, cubeSize), Vec(cubeSize,-cubeSize,cubeSize), Vec(cubeSize,-cubeSize,-cubeSize),
														Vec(-cubeSize,cubeSize,-cubeSize), Vec(-cubeSize,cubeSize, cubeSize), Vec(cubeSize,cubeSize,cubeSize), Vec(cubeSize,cubeSize,-cubeSize) };
		PolyIds init_cube_polys = { 0,1,2,3,4,5,6,7 };
		Utils::Geometry::sortVids(init_cube_polys, init_cube_coords);

		mesh.clear();
		mesh.init(
			std::vector<Vec>{ init_cube_coords.begin(), init_cube_coords.end() },
			std::vector<std::vector<Id>>{ std::vector<Id>{init_cube_polys.begin(), init_cube_polys.end()} }
		);

		mesh.poly_data(0).element = op_tree.root;
		op_tree.root->pid() = 0;

		//update displacement
		for (Id off = 0; off < 8; off++)
		{
			op_tree.move(*op_tree.root, off, mesh.poly_vert(0, off));
		}

		v_map.clear();
		m_commander.clear();
		refine_queue.clear();

		for (Id i = 0; i < 8; i++)
		{
			v_map[init_cube_coords[i]] = i;
		}

		update_mesh();


	}

	void Grid::move_vert(Id vid, const Vec& displacement)
	{

		add_move(vid, displacement);
	}

	void Grid::move_edge(Id eid, const Vec& displacement)
	{
		for (Id vid : mesh.adj_e2v(eid))
		{

			add_move(vid, displacement);

		}
		update_mesh();
	}

	void Grid::move_face(Id fid, const Vec& displacement)
	{

		for (Id vid : mesh.adj_f2v(fid))
		{

			add_move(vid, displacement);

		}
		update_mesh();
	}

	//ADD OPERATIONS############################################################################################################################


	void Grid::add_refine(Id pid)
	{
		m_commander.apply(*new Actions::Refine(mesh.poly_centroid(pid), mesh.face_centroid(mesh.poly_faces_id(pid)[0]), Refinement::EScheme::StandardRefinement));
	}

	void Grid::add_face_refine(Id fid)
	{
		m_commander.apply(*new Actions::Refine(mesh.poly_centroid(mesh.adj_f2p(fid).front()), mesh.face_centroid(fid), Refinement::EScheme::FaceRefinement));
	}

	void Grid::add_extrude(Id pid, Id face_offset)
	{
		m_commander.apply(*new Actions::Extrude(mesh.poly_centroid(pid), mesh.face_centroid(mesh.poly_faces_id(pid)[face_offset])));
	}

	void Grid::add_move(Id vid, const Vec& displacement)
	{
		m_commander.apply(*new Actions::MoveVert(mesh.vert(vid), displacement));
	}

	void Grid::add_remove(Id pid)
	{
		m_commander.apply(*new Actions::Delete(mesh.poly_centroid(pid)));
	}

	//REMOVE OPERATIONS############################################################################################################################

	void Grid::undo()
	{
		m_commander.undo();
		update_mesh();

	}

	void Grid::redo()
	{
		m_commander.redo();
		update_mesh();
	}

	void Grid::prune_tree(const Dag::Operation& operation, bool is_user_defined)
	{

		/*    std::vector<Id> polys_to_remove;
			remove_operation(operation, polys_to_remove);

			for(auto &node : operation->parents){
				auto parent = std::static_pointer_cast<Element>(node);
				parent->operations.remove(operation);

			}

			operation->parents.clear();

			std::sort(polys_to_remove.begin(), polys_to_remove.end(), std::greater());
			for(Id pid : polys_to_remove) mesh.poly_remove(pid, false);

			update_mesh();*/

	}


	void Grid::apply_tree_recursive(const std::vector<Dag::Operation*>& operations, Id pid, bool is_user_defined)
	{
		throw std::logic_error{ "not implemented" };
		for (auto op : operations)
		{
			switch (op->primitive())
			{
				case Dag::Operation::EPrimitive::Refine:
				{
					auto re = static_cast<Dag::Refine*>(op);
					if (!is_user_defined) refine_queue.push_back(re);
					refine(pid, *re, true);
					break;
				}
				case Dag::Operation::EPrimitive::Extrude:
				{
					auto ex = static_cast<Dag::Extrude*>(op);
					extrude(pid, ex->faceOffset(), *ex);
					break;
				}

				case Dag::Operation::EPrimitive::Delete:
					removePoly(pid);
					break;
			}

			for (auto& child : op->children())
			{
				apply_tree_recursive(child.children().vector(), child.pid(), false);
			}

		}



	}



	void Grid::apply_tree(const std::vector < Dag::Operation* >& operations, Id pid, bool is_user_defined)
	{

		apply_tree_recursive(operations, pid, is_user_defined);


		for (Id pid = 0; pid < mesh.num_polys(); pid++)
		{
			for (Id off = 0; off < 8; off++)
			{
				Id vid = mesh.poly_vert_id(pid, off);
				const auto& el = mesh.poly_data(pid).element;
				move(vid, el->vertices()[off] - mesh.vert(vid));
			}
		}

		update_mesh();
	}

	void Grid::update_displacement_for_op(Dag::Operation& op)
	{
		for (auto& child : op.children())
		{

			Id pid = child.pid();

			//update displacement
			for (Id off = 0; off < 8; off++)
			{
				op_tree.move(child, off, mesh.poly_vert(pid, off));
			}
		}
	}

	void Grid::make_conforming()
	{
		m_commander.apply(*new MakeConformingAction(*this));
		update_mesh();
	}

	void Grid::project_on_target(cinolib::Trimesh<>& target)
	{
		cinolib::Quadmesh<> peel;
		std::unordered_map<Id, Id> h2q, q2h;
		cinolib::export_surface(mesh, peel, h2q, q2h);

		std::unordered_map<Id, Id> m_map;
		std::unordered_map<Id, Id> v_map;
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

		Id count = 0;
		for (Id vid = 0; vid < peel.num_verts(); vid++)
		{
			if (!(mesh.vert(v_map[vid]) == peel.vert(vid)))
			{
				auto displacement = peel.vert(vid) - mesh.vert(v_map[vid]);
				add_move(v_map[vid], displacement);
				count++;
			}
		}
		// TODO collapse
		//command_manager.collapse_last_n_actions(count); 

		update_mesh();

		std::cout << "Projection completed" << std::endl;

	}

	bool Grid::merge(Id pid_source, Id pid_dest)
	{

		std::vector<Dag::Operation*> new_ops;
		auto& source_el = *mesh.poly_data(pid_source).element;
		auto& destination_el = *mesh.poly_data(pid_dest).element;

		Vec centroid_dest = mesh.poly_centroid(pid_dest);

		if (op_tree.merge(source_el, destination_el, &new_ops))
		{

			std::vector<const Dag::Operation*> branch_ops;
			for (auto& op : new_ops) op_tree.get_branch_operations(*op, branch_ops);
			//The offset of the first extrude primitive must be changed to reflect the destination node local frame
			//If the first primitive is a rfine, then its vertex map must be rotated to reflect the destination node local frame
			//for(auto &op : branch_ops) apply_transform(op, ROTATE_Y);
			apply_tree_recursive(new_ops, pid_dest);

			return true;

			std::map<Id, Vec> vert_map;
			std::vector<const Dag::Element*> old_elements;
			std::vector<const Dag::Element*> new_elements;

			op_tree.get_branch_elements(source_el, old_elements);

			for (const auto& op : new_ops) op_tree.get_branch_elements(*op, new_elements);

			assert(old_elements.size() == new_elements.size());

			for (Id i = 0; i < old_elements.size(); i++)
			{
				const auto& el_old = *old_elements[i];
				const auto& el_new = *new_elements[i];
				Id pid = el_old.pid();
				Id new_pid = el_new.pid();
				auto& new_el = *mesh.poly_data(new_pid).element;
				for (Id off = 0; off < 8; off++)
				{
					Id vid = mesh.poly_vert_id(pid, off);
					Id vid_dest = mesh.poly_vert_id(new_pid, off);
					if (mesh.poly_contains_vert(pid_dest, vid_dest)) continue;
					vert_map[vid_dest] = mesh.vert(vid) - mesh.poly_centroid(pid_source);
					new_el.vertices()[off] = centroid_dest + vert_map[vid_dest];
				}
			}


			for (const auto& pair : vert_map)
			{
				Id vid = pair.first;
				Vec displ = pair.second;
				Vec final_displ = -mesh.vert(vid) + centroid_dest + displ;
				final_displ.rotate(Vec(0, 1, 0), pi / 2);
				move(vid, -mesh.vert(vid) + centroid_dest + displ);
			}


			return true;
		}
		return false;
	}

	void Grid::clear()
	{
		init();
	}

	void Grid::save_as_mesh(std::string filename)
	{
		std::vector<std::vector<Id>> polys;

		for (Id pid = 0; pid < mesh.num_polys(); pid++)
		{
			if (!mesh.poly_data(pid).flags[cinolib::HIDDEN])
			{
				polys.push_back(mesh.poly_verts_id(pid));
			}
		}

		cinolib::Hexmesh<> m(mesh.vector_verts(), polys);

		m.save(filename.c_str());
	}

	void Grid::extrude(Id _pid, Id _offset, Dag::Extrude& _operation)
	{
		const Id fid = mesh.poly_face_id(_pid, _offset);

		Real avgFaceEdgeLength{};
		{
			const std::vector<Id> faceEids{ mesh.adj_f2e(fid) };
			for (const Id eid : faceEids)
			{
				avgFaceEdgeLength += mesh.edge_length(eid);
			}
			avgFaceEdgeLength /= mesh.adj_f2e(fid).size();
		}

		const std::vector<Vec> faceVerts = mesh.face_verts(fid);
		const Vec faceNormal = mesh.face_data(fid).normal;
		PolyIds vids;
		{
			const std::vector<Id> faceVids{ mesh.face_verts_id(fid) };
			std::copy(faceVids.begin(), faceVids.end(), vids.begin());
		}
		int i{ 4 };
		for (const Vec& faceVert : faceVerts)
		{
			const Vec vert{ faceVert + faceNormal * avgFaceEdgeLength };
			vids[i++] = addOrGetVert(vert);
		}

		const Id pid = mesh.poly_add(std::vector<Id>{vids.begin(), vids.end() });
		Dag::Element& element{ _operation.children().single() };
		element.pid() = pid;
		mesh.poly_data(pid).element = &element;
	}

	void Grid::refine(Id pid, Dag::Refine& refine, bool remove_father)
	{

		const auto& weights = Refinement::schemes.at(refine.scheme())->weights;
		const auto& vids = Refinement::schemes.at(refine.scheme())->offsets;

		auto childrenIterator{ refine.children().begin() };

		for (Id i = 0; i < weights.size(); i++)
		{

			std::vector<Id> poly;

			for (Id j = 0; j < weights[i].size(); j++)
			{
				const auto& tmp = weights[i][j];
				Vec vert(0, 0, 0);
				for (Id k = 0; k < 4; k++)
				{
					//Id off = refine->vert_map.empty() ? vids[i][j][k] : refine->vert_map[vids[i][j][k]];
					Id off = refine.vertices()[vids[i][j][k]];

					vert += tmp[k] * mesh.poly_vert(pid, off);
				}

				auto query = v_map.find(vert);
				Id vid = query == v_map.end() ? mesh.vert_add(vert) : query->second;
				if (query == v_map.end()) v_map[vert] = vid;

				poly.push_back(vid);
			}


			//poly_vert_ordering(mesh.vector_verts(), poly);
			Id new_pid = mesh.poly_add(poly);

			auto& element = *childrenIterator;
			element.pid() = new_pid;
			mesh.poly_data(new_pid).element = &element;
			childrenIterator++;
		}
		if (remove_father) mesh.poly_remove(pid, false);
		else 			  mesh.poly_data(pid).flags[cinolib::HIDDEN] = true;

		update_mesh();

	}

	void Grid::move(Id vid, const Vec& displacement)
	{
		v_map.erase(mesh.vert(vid));
		mesh.vert(vid) += displacement;
		v_map[mesh.vert(vid)] = vid;
		update_mesh();
	}

	void Grid::removePoly(Id _pid)
	{
		mesh.poly_remove(_pid, true);
		if (_pid < mesh.num_polys())
		{
			element(_pid).pid() = _pid;
		}
	}

	void Grid::update_mesh()
	{
		mesh.updateGL();
	}

	Id Grid::addPoly(const PolyVerts _vertices, Dag::Element& _element)
	{
		PolyIds vids;
		for (size_t i{ 0 }; i < 8; i++)
		{
			vids[i] = addOrGetVert(_vertices[i]);
		}
		return addPoly(vids, _element);
	}

	Id Grid::addOrGetVert(const Vec& _vert)
	{
		Id vid;
		if (!getVert(_vert, vid))
		{
			vid = mesh.vert_add(_vert);
		}
		return vid;
	}

	bool Grid::getVert(const Vec& _vert, Id& _vid) const
	{
		_vid = mesh.pick_vert(_vert);
		return mesh.vert(_vid).dist(_vert) <= c_maxVertDistance;
	}

	Id Grid::getVert(const Vec& _vert) const
	{
		Id vid;
		if (!getVert(_vert, vid))
		{
			throw std::logic_error{ "not found" };
		}
		return vid;
	}

	bool Grid::hasVert(const Vec& _vert) const
	{
		Id vid;
		return getVert(_vert, vid);
	}

	Dag::Element& Grid::element(Id _pid)
	{
		return *mesh.poly_data(_pid).element;
	}

	void Grid::element(Id _pid, Dag::Element& _element)
	{
		mesh.poly_data(_pid).element = &_element;
		_element.pid() = _pid;
	}

	const Dag::Element& Grid::element(Id _pid) const
	{
		return const_cast<Grid*>(this)->element(_pid);
	}

	PolyVerts Grid::polyVerts(Id _pid) const
	{
		return Utils::Collections::toArray<8>(mesh.poly_verts(_pid));
	}

	PolyVerts Grid::polyVerts(Id _pid, Id _faceOffset) const
	{
		const Id frontFid{ mesh.poly_faces_id(_pid)[_faceOffset] };
		const Id backFid{ mesh.poly_face_opposite_to(_pid, frontFid) };
		std::vector<Id> frontFaceVids = mesh.face_verts_id(frontFid);
		std::vector<Id> backFaceVids = mesh.face_verts_id(backFid);

		if (mesh.poly_face_winding(_pid, frontFid))
		{
			std::reverse(frontFaceVids.begin(), frontFaceVids.end());
		}
		if (!mesh.poly_face_winding(_pid, backFid))
		{
			std::reverse(backFaceVids.begin(), backFaceVids.end());
		}

		while (mesh.edge_id(frontFaceVids[0], backFaceVids[0]) == -1)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}

		PolyVerts verts;
		size_t i{ 0 };
		for (const Id vid : frontFaceVids)
		{
			verts[i++] = mesh.vert(vid);
		}
		for (const Id vid : backFaceVids)
		{
			verts[i++] = mesh.vert(vid);
		}

		return verts;
	}

	Id Grid::addPoly(Dag::Element& _element)
	{
		return addPoly(_element.vertices(), _element);
	}

	Id Grid::addPoly(const PolyIds& _vids, Dag::Element& _element)
	{
		const Id pid{ mesh.poly_add(std::vector<Id>{_vids.begin(), _vids.end()}) };
		element(pid, _element);
		_element.vertices() = polyVerts(pid);
		return pid;
	}

	void Grid::vert(Id _vid, const Vec& _position)
	{
		Id closestVid;
		if (getVert(_position, closestVid) && closestVid != _vid)
		{
			throw std::logic_error{ "move will result in vertex merge" };
		}
		for (const Id pid : mesh.adj_v2p(_vid))
		{
			const Id offset{ mesh.poly_vert_offset(pid, _vid) };
			element(pid).vertices()[offset] = _position;
		}
		mesh.vert(_vid) = _position;
	}

	Id Grid::closestPolyFid(Id _pid, const Vec& _centroid) const
	{
		Real closestDist{ cinolib::inf_double };
		Id closestFid{};
		for (Id fid : mesh.poly_faces_id(_pid))
		{
			const Real dist{ _centroid.dist(mesh.face_centroid(fid)) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestFid = fid;
			}
		}
		return closestFid;
	}

}