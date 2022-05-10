#include <HMP/Meshing/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <stdexcept>
#include <cpputils/collections/conversions.hpp>

namespace HMP::Meshing::Utils
{

	Id anyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid)
	{
		if (!_mesh.poly_contains_edge(_pid, _eid))
		{
			throw std::logic_error{ "edge not in poly" };
		}
		for (const Id fid : _mesh.adj_e2f(_eid))
		{
			if (_mesh.poly_contains_face(_pid, fid))
			{
				return fid;
			}
		}
		throw std::runtime_error{ "unexpected" };
	}

	Id adjacentFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid)
	{
		if (!_mesh.poly_contains_face(_pid, _fid))
		{
			throw std::logic_error{ "face not in poly" };
		}
		if (!_mesh.face_contains_edge(_fid, _eid))
		{
			throw std::logic_error{ "edge not in face" };
		}
		for (const Id fid : _mesh.poly_faces_id(_pid))
		{
			if (fid != _fid && _mesh.face_shared_edge(fid, _fid) == _eid)
			{
				return fid;
			}
		}
		throw std::runtime_error{ "unexpected" };
	}

	FaceVertIds faceVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _winding)
	{
		if (!_mesh.poly_contains_face(_pid, _fid))
		{
			throw std::logic_error{ "face not in poly" };
		}
		std::vector<Id> vids{ _mesh.face_verts_id(_fid) };
		if (_winding != _mesh.poly_face_winding(_pid, _fid))
		{
			std::reverse(vids.begin(), vids.end());
		}
		return cpputils::collections::conversions::toArray<4>(vids);
	}

	FaceVertIds faceVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _upEid, bool _winding)
	{
		if (!_mesh.face_contains_edge(_fid, _upEid))
		{
			throw std::logic_error{ "edge not in face" };
		}
		FaceVertIds vids{ faceVids(_mesh, _pid, _fid, _winding) };
		while (_mesh.edge_id(vids[0], vids[1]) != _upEid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.end());
		}
		return cpputils::collections::conversions::toArray<4>(vids);
	}

	PolyVertIds polyVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid)
	{
		const FaceVertIds forwardFaceVids{ faceVids(_mesh, _pid, _forwardFid, _forwardUpEid, false) };
		FaceVertIds backFaceVids{ faceVids(_mesh, _pid, _mesh.poly_face_opposite_to(_pid, _forwardFid), true) };
		while (_mesh.edge_id(forwardFaceVids[0], backFaceVids[0]) == noId)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}
		PolyVertIds vids;
		std::copy(forwardFaceVids.begin(), forwardFaceVids.end(), vids.begin());
		std::copy(backFaceVids.begin(), backFaceVids.end(), vids.begin() + 4);
		return vids;
	}

	FaceVerts verts(const Meshing::Mesher::Mesh& _mesh, const FaceVertIds& _vids)
	{
		FaceVerts verts;
		for (std::size_t i{}; i < 4; i++)
		{
			verts[i] = _mesh.vert(_vids[i]);
		}
		return verts;
	}

	PolyVerts verts(const Meshing::Mesher::Mesh& _mesh, const PolyVertIds& _vids)
	{
		PolyVerts verts;
		for (std::size_t i{}; i < 8; i++)
		{
			verts[i] = _mesh.vert(_vids[i]);
		}
		return verts;
	}

	Vec midpoint(const Meshing::Mesher::Mesh& _mesh, Id _eid)
	{
		const std::vector<Vec> verts{ _mesh.edge_verts(_eid) };
		return (verts[0] + verts[1]) / 2;
	}

	Id closestPolyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestFid{};
		for (const Id fid : _mesh.poly_faces_id(_pid))
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

	Id closestFaceVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _position)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestVid{};
		for (const Id vid : _mesh.face_verts_id(_fid))
		{
			const Real dist{ _position.dist(_mesh.vert(vid)) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestVid = vid;
			}
		}
		return closestVid;
	}

	Id closestFaceEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _midpoint)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestEid{};
		for (Id edgeOffset{ 0 }; edgeOffset < 4; edgeOffset++)
		{
			const Id eid{ _mesh.face_edge_id(_fid, edgeOffset) };
			const Vec midpoint{ Utils::midpoint(_mesh, eid) };
			const Real dist{ _midpoint.dist(midpoint) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestEid = eid;
			}
		}
		return closestEid;
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