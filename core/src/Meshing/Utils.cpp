#include <HMP/Meshing/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <stdexcept>
#include <cpputils/collections/conversions.hpp>

namespace HMP::Meshing::Utils
{

	std::pair<Id, Id> adjacentFidsFromEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid)
	{
		bool foundFirst{ false };
		std::pair<Id, Id> fids;
		for (const Id fid : _mesh.poly_faces_id(_pid))
		{
			if (_mesh.face_contains_edge(fid, _eid))
			{
				(foundFirst ? fids.first : fids.second) = fid;
				foundFirst = true;
			}
		}
		return fids;
	}

	Id anyAdjacentFidFromFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid)
	{
		if (!_mesh.poly_contains_face(_pid, _fid))
		{
			throw std::logic_error{ "face not in poly" };
		}
		for (const Id otherFid : _mesh.poly_faces_id(_pid))
		{
			if (otherFid != _fid && _mesh.face_shared_edge(_fid, otherFid) != noId)
			{
				return otherFid;
			}
		}
		throw std::runtime_error{ "unexpected" };
	}

	Id anyAdjacentFaceOffsetFromFaceOffset(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _faceOffset)
	{
		return _mesh.poly_face_offset(_pid, anyAdjacentFidFromFid(_mesh, _pid, _mesh.poly_face_id(_pid, _faceOffset)));
	}

	FaceVertIds faceVidsFromFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _winding)
	{
		if (!_mesh.poly_contains_face(_pid, _fid))
		{
			throw std::logic_error{ "face not in poly" };
		}
		std::vector<Id> vids = _mesh.face_verts_id(_fid);
		if (_winding != _mesh.poly_face_winding(_pid, _fid))
		{
			std::reverse(vids.begin(), vids.end());
		}
		return cpputils::collections::conversions::toArray<4>(vids);
	}

	FaceVertIds faceVidsFromFaceOffset(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _faceOffset, bool _winding)
	{
		return faceVidsFromFid(_mesh, _pid, _mesh.poly_face_id(_pid, _faceOffset), _winding);
	}

	PolyVerts polyVertsFromFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _upFid)
	{
		if (!(_mesh.poly_contains_face(_pid, _forwardFid) && _mesh.poly_contains_face(_pid, _upFid)))
		{
			throw std::logic_error{ "face not in poly" };
		}
		const Id sharedEid{ _mesh.face_shared_edge(_forwardFid, _upFid) };
		if (sharedEid == noId)
		{
			throw std::logic_error{ "faces are not adjacent" };
		}
		const Id backFid{ _mesh.poly_face_opposite_to(_pid, _forwardFid) };
		FaceVertIds forwardFaceVids = faceVidsFromFid(_mesh, _pid, _forwardFid, true);
		FaceVertIds backFaceVids = faceVidsFromFid(_mesh, _pid, backFid, false);
		while (_mesh.edge_id(forwardFaceVids[0], forwardFaceVids[1]) != sharedEid)
		{
			std::rotate(forwardFaceVids.begin(), forwardFaceVids.begin() + 1, forwardFaceVids.end());
		}
		while (_mesh.edge_id(forwardFaceVids[0], backFaceVids[0]) == noId)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}
		PolyVerts verts;
		size_t i{ 0 };
		for (const Id vid : forwardFaceVids)
		{
			verts[i++] = _mesh.vert(vid);
		}
		for (const Id vid : backFaceVids)
		{
			verts[i++] = _mesh.vert(vid);
		}

		return verts;
	}

	PolyVerts polyVertsFromFaceOffsets(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFaceOffset, Id _upFaceOffset)
	{
		return polyVertsFromFids(_mesh, _pid, _mesh.poly_face_id(_pid, _forwardFaceOffset), _mesh.poly_face_id(_pid, _upFaceOffset));
	}

	Id closestFaceOffsetInPoly(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestOffset{};
		for (Id faceOffset{ 0 }; faceOffset < 6; faceOffset++)
		{
			const Real dist{ _centroid.dist(_mesh.face_centroid(_mesh.poly_face_id(_pid, faceOffset))) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestOffset = faceOffset;
			}
		}
		return closestOffset;
	}

	Id closestVertOffsetInPoly(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _position)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestOffset{};
		for (Id vertOffset{ 0 }; vertOffset < 8; vertOffset++)
		{
			const Real dist{ _position.dist(_mesh.vert(_mesh.poly_vert_id(_pid, vertOffset))) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestOffset = vertOffset;
			}
		}
		return closestOffset;
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