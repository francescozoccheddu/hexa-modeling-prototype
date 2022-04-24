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
		m_mesh.show_mesh_flat();
		clear();
	}

	Commander& Grid::commander()
	{
		return m_commander;
	}


	const Commander& Grid::commander() const
	{
		return m_commander;
	}

	//ADD OPERATIONS############################################################################################################################


	void Grid::add_refine(Id pid)
	{
		m_commander.apply(*new Actions::Refine(m_mesh.poly_centroid(pid), m_mesh.face_centroid(m_mesh.poly_faces_id(pid)[0]), Refinement::EScheme::Subdivide3x3));
	}

	void Grid::add_face_refine(Id fid)
	{
		m_commander.apply(*new Actions::Refine(m_mesh.poly_centroid(m_mesh.adj_f2p(fid).front()), m_mesh.face_centroid(fid), Refinement::EScheme::Inset));
	}

	void Grid::add_extrude(Id pid, Id face_offset)
	{
		m_commander.apply(*new Actions::Extrude(m_mesh.poly_centroid(pid), m_mesh.face_centroid(m_mesh.poly_faces_id(pid)[face_offset])));
	}

	void Grid::add_move(Id vid, const Vec& displacement)
	{
		m_commander.apply(*new Actions::MoveVert(m_mesh.vert(vid), displacement));
	}

	void Grid::add_remove(Id pid)
	{
		m_commander.apply(*new Actions::Delete(m_mesh.poly_centroid(pid)));
	}

	//REMOVE OPERATIONS############################################################################################################################

	void Grid::undo()
	{
		m_commander.undo();
		m_mesh.updateGL();

	}

	void Grid::redo()
	{
		m_commander.redo();
		m_mesh.updateGL();
	}

	void Grid::make_conforming()
	{
		m_commander.apply(*new Actions::MakeConforming());
		m_mesh.updateGL();
	}

	void Grid::project_on_target(cinolib::Trimesh<>& target)
	{
		cinolib::Quadmesh<> peel;
		std::unordered_map<Id, Id> h2q, q2h;
		cinolib::export_surface(m_mesh, peel, h2q, q2h);

		std::unordered_map<Id, Id> m_map;
		std::unordered_map<Id, Id> v_map;
		cinolib::export_surface(m_mesh, peel, m_map, v_map);
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
			if (!(m_mesh.vert(v_map[vid]) == peel.vert(vid)))
			{
				auto displacement = peel.vert(vid) - m_mesh.vert(v_map[vid]);
				add_move(v_map[vid], displacement);
				count++;
			}
		}
		// TODO collapse
		//command_manager.collapse_last_n_actions(count); 

		m_mesh.updateGL();

		std::cout << "Projection completed" << std::endl;

	}

	bool Grid::merge(Id pid_source, Id pid_dest)
	{
		return false;
	}

	void Grid::clear()
	{
		Dag::Element& root{ *new Dag::Element{} };
		root.vertices() = {
			Vec(-cubeSize,-cubeSize,-cubeSize), Vec(-cubeSize,-cubeSize, cubeSize), Vec(cubeSize,-cubeSize,cubeSize), Vec(cubeSize,-cubeSize,-cubeSize),
			Vec(-cubeSize,cubeSize,-cubeSize), Vec(-cubeSize,cubeSize, cubeSize), Vec(cubeSize,cubeSize,cubeSize), Vec(cubeSize,cubeSize,-cubeSize)
		};
		replaceDag(root);
	}

	void Grid::save_as_mesh(std::string filename)
	{
		std::vector<std::vector<Id>> polys;

		for (Id pid = 0; pid < m_mesh.num_polys(); pid++)
		{
			if (!m_mesh.poly_data(pid).flags[cinolib::HIDDEN])
			{
				polys.push_back(m_mesh.poly_verts_id(pid));
			}
		}

		cinolib::Hexmesh<> m(m_mesh.vector_verts(), polys);

		m.save(filename.c_str());
	}

	void Grid::removePoly(Id _pid)
	{
		m_mesh.poly_remove(_pid, true);
		if (_pid < m_mesh.num_polys())
		{
			element(_pid).pid() = _pid;
		}
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
			vid = m_mesh.vert_add(_vert);
		}
		return vid;
	}

	bool Grid::getVert(const Vec& _vert, Id& _vid) const
	{
		if (!m_mesh.num_verts())
		{
			return false;
		}
		_vid = m_mesh.pick_vert(_vert);
		return m_mesh.vert(_vid).dist(_vert) <= c_maxVertDistance;
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
		return *m_mesh.poly_data(_pid).element;
	}

	void Grid::element(Id _pid, Dag::Element& _element)
	{
		m_mesh.poly_data(_pid).element = &_element;
		_element.pid() = _pid;
	}

	const Dag::Element& Grid::element(Id _pid) const
	{
		return const_cast<Grid*>(this)->element(_pid);
	}

	PolyVerts Grid::polyVerts(Id _pid) const
	{
		return Utils::Collections::toArray<8>(m_mesh.poly_verts(_pid));
	}

	PolyVerts Grid::polyVertsFromFace(Id _pid, Id _faceOffset) const
	{
		const Id frontFid{ m_mesh.poly_faces_id(_pid)[_faceOffset] };
		const Id backFid{ m_mesh.poly_face_opposite_to(_pid, frontFid) };
		std::vector<Id> frontFaceVids = m_mesh.face_verts_id(frontFid);
		std::vector<Id> backFaceVids = m_mesh.face_verts_id(backFid);

		if (m_mesh.poly_face_winding(_pid, frontFid))
		{
			std::reverse(frontFaceVids.begin(), frontFaceVids.end());
		}
		if (!m_mesh.poly_face_winding(_pid, backFid))
		{
			std::reverse(backFaceVids.begin(), backFaceVids.end());
		}

		while (m_mesh.edge_id(frontFaceVids[0], backFaceVids[0]) == -1)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}

		PolyVerts verts;
		size_t i{ 0 };
		for (const Id vid : frontFaceVids)
		{
			verts[i++] = m_mesh.vert(vid);
		}
		for (const Id vid : backFaceVids)
		{
			verts[i++] = m_mesh.vert(vid);
		}

		return verts;
	}

	PolyVerts Grid::polyVertsFromEdge(Id _pid, Id _eid) const
	{
		Id fid = 0;
		for (Id edgeFid : m_mesh.adj_e2f(_eid))
		{
			if (m_mesh.poly_contains_face(_pid, edgeFid))
			{
				fid = edgeFid;
				break;
			}
		}
		const Id frontFid{ fid };
		const Id backFid{ m_mesh.poly_face_opposite_to(_pid, frontFid) };
		std::vector<Id> frontFaceVids = m_mesh.face_verts_id(frontFid);
		std::vector<Id> backFaceVids = m_mesh.face_verts_id(backFid);

		if (m_mesh.poly_face_winding(_pid, frontFid))
		{
			std::reverse(frontFaceVids.begin(), frontFaceVids.end());
		}
		if (!m_mesh.poly_face_winding(_pid, backFid))
		{
			std::reverse(backFaceVids.begin(), backFaceVids.end());
		}

		while ((frontFaceVids[0] + frontFaceVids[1]) - (m_mesh.edge_vert_id(_eid, 0) + m_mesh.edge_vert_id(_eid, 1)) != 0)
		{
			std::rotate(frontFaceVids.begin(), frontFaceVids.begin() + 1, frontFaceVids.end());
		}

		while (m_mesh.edge_id(frontFaceVids[0], backFaceVids[0]) == -1)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}

		PolyVerts verts;
		size_t i{ 0 };
		for (const Id vid : frontFaceVids)
		{
			verts[i++] = m_mesh.vert(vid);
		}
		for (const Id vid : backFaceVids)
		{
			verts[i++] = m_mesh.vert(vid);
		}

		return verts;
	}

	Id Grid::addPoly(Dag::Element& _element)
	{
		return addPoly(_element.vertices(), _element);
	}

	Id Grid::addPoly(const PolyIds& _vids, Dag::Element& _element)
	{
		const Id pid{ m_mesh.poly_add(std::vector<Id>{_vids.begin(), _vids.end()}) };
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
		for (const Id pid : m_mesh.adj_v2p(_vid))
		{
			const Id offset{ m_mesh.poly_vert_offset(pid, _vid) };
			element(pid).vertices()[offset] = _position;
		}
		m_mesh.vert(_vid) = _position;
	}

	Id Grid::closestPolyFid(Id _pid, const Vec& _centroid) const
	{
		Real closestDist{ cinolib::inf_double };
		Id closestFid{};
		for (Id fid : m_mesh.poly_faces_id(_pid))
		{
			const Real dist{ _centroid.dist(m_mesh.face_centroid(fid)) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestFid = fid;
			}
		}
		return closestFid;
	}

	Dag::Element& Grid::root() const
	{
		return *m_root;
	}

	void Grid::replaceDag(Dag::Element& _root)
	{
		if (m_root)
		{
			delete m_root;
		}
		m_mesh.clear();
		m_commander.clear();

		m_root = &_root;
		for (Dag::Node* node : Dag::Utils::descendants(_root))
		{
			if (node->isElement())
			{
				Dag::Element& element{ node->element() };
				if (element.children().all([](const Dag::Operation& _child) {return _child.primitive() == Dag::Operation::EPrimitive::Extrude; }))
				{
					addPoly(element);
				}
			}
		}
		m_mesh.updateGL();
	}

	Grid::Mesh& Grid::mesh()
	{
		return m_mesh;
	}

	const Grid::Mesh& Grid::mesh() const
	{
		return m_mesh;
	}

}