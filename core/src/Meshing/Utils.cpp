#include <HMP/Meshing/Utils.hpp>

namespace HMP::Meshing::Utils
{

	PolyVerts polyVertsFromFace(const Mesher::Mesh& _mesh, Id _pid, Id _fid)
	{
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

	PolyVerts polyVertsFromEdge(const Mesher::Mesh& _mesh, Id _pid, Id _eid)
	{
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

}