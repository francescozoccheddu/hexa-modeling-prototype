#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <utility>

namespace HMP::Meshing::Utils
{

	std::pair<Id, Id> adjacentFidsFromEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid);
	Id anyAdjacentFidFromFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid);
	Id anyAdjacentFaceOffsetFromFaceOffset(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _faceOffset);
	FaceVertIds faceVidsFromFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _winding = false);
	FaceVertIds faceVidsFromFaceOffset(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _faceOffset, bool _winding = false);
	PolyVerts polyVertsFromFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFaceOffset, Id _upFaceOffset);
	PolyVerts polyVertsFromFaceOffsets(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _upFid);

	Id closestFaceOffsetInPoly(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid);
	Id closestVertOffsetInPoly(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _position);

	void addLeafs(Mesher& _mesher, Dag::Element& _root, bool _clear = true);

}