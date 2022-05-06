#include <HMP/Meshing/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <stdexcept>

namespace HMP::Meshing::Utils
{

	PolyVerts polyVertsFromFace(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid)
	{
		if (!_mesh.poly_contains_face(_pid, _fid))
		{
			throw std::logic_error{ "face not in poly" };
		}

		const Id frontFid{ _fid };
		const Id backFid{ _mesh.poly_face_opposite_to(_pid, frontFid) };
		std::vector<Id> frontFaceVids = _mesh.face_verts_id(frontFid);
		std::vector<Id> backFaceVids = _mesh.face_verts_id(backFid);

		if (_mesh.poly_face_winding(_pid, frontFid))
		{
			std::reverse(frontFaceVids.begin(), frontFaceVids.end());
		}
		if (!_mesh.poly_face_winding(_pid, backFid))
		{
			std::reverse(backFaceVids.begin(), backFaceVids.end());
		}

		while (_mesh.edge_id(frontFaceVids[0], backFaceVids[0]) == -1)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}

		PolyVerts verts;
		size_t i{ 0 };
		for (const Id vid : frontFaceVids)
		{
			verts[i++] = _mesh.vert(vid);
		}
		for (const Id vid : backFaceVids)
		{
			verts[i++] = _mesh.vert(vid);
		}

		return verts;
	}

	PolyVerts polyVertsFromEdge(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid)
	{
		if (!_mesh.poly_contains_edge(_pid, _eid))
		{
			throw std::logic_error{ "edge not in poly" };
		}

		Id fid = 0;
		for (Id edgeFid : _mesh.adj_e2f(_eid))
		{
			if (_mesh.poly_contains_face(_pid, edgeFid))
			{
				fid = edgeFid;
				break;
			}
		}
		const Id frontFid{ fid };
		const Id backFid{ _mesh.poly_face_opposite_to(_pid, frontFid) };
		std::vector<Id> frontFaceVids = _mesh.face_verts_id(frontFid);
		std::vector<Id> backFaceVids = _mesh.face_verts_id(backFid);

		if (_mesh.poly_face_winding(_pid, frontFid))
		{
			std::reverse(frontFaceVids.begin(), frontFaceVids.end());
		}
		if (!_mesh.poly_face_winding(_pid, backFid))
		{
			std::reverse(backFaceVids.begin(), backFaceVids.end());
		}

		while ((frontFaceVids[0] + frontFaceVids[1]) - (_mesh.edge_vert_id(_eid, 0) + _mesh.edge_vert_id(_eid, 1)) != 0)
		{
			std::rotate(frontFaceVids.begin(), frontFaceVids.begin() + 1, frontFaceVids.end());
		}

		while (_mesh.edge_id(frontFaceVids[0], backFaceVids[0]) == -1)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}

		PolyVerts verts;
		size_t i{ 0 };
		for (const Id vid : frontFaceVids)
		{
			verts[i++] = _mesh.vert(vid);
		}
		for (const Id vid : backFaceVids)
		{
			verts[i++] = _mesh.vert(vid);
		}

		return verts;
	}

	Id closestFaceInPoly(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestFid{};
		for (Id fid : _mesh.poly_faces_id(_pid))
		{
			const Real dist{ _centroid.dist(_mesh.face_centroid(fid)) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestFid = fid;
			}
		}
		return closestFid;
	}

	void addLeafs(Mesher& _mesher, Dag::Element& _root, bool _clear)
	{
		if (_clear)
		{
			_mesher.clear();
		}

		for (Dag::Node* node : Dag::Utils::descendants(_root))
		{
			if (node->isElement())
			{
				Dag::Element& element{ node->element() };
				bool active{ true };
				for (const Dag::Operation& child : element.children())
				{
					if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
					{
						active = false;
					}
				}
				if (active)
				{
					_mesher.add(element);
				}
			}
		}
	}

}