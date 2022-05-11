#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <utility>

namespace HMP::Meshing::Utils
{

	Id rotateEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, Id _eid, int _rotation);

	Id anyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid);
	Id adjacentFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid);
	
	FaceVertIds faceVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _winding = false);
	FaceVertIds faceVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _upEid, bool _winding = false);
	PolyVertIds polyVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid);

	FaceVerts verts(const Meshing::Mesher::Mesh& _mesh, const FaceVertIds& _vids);
	PolyVerts verts(const Meshing::Mesher::Mesh& _mesh, const PolyVertIds& _vids);
	Vec midpoint(const Meshing::Mesher::Mesh& _mesh, Id _eid);

	Id closestPolyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid);
	Id closestFaceEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _midpoint);
	Id closestFaceVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _position);

	void addLeafs(Mesher& _mesher, Dag::Node& _root, bool _clear = true);
	void removeLeafs(Mesher& _mesher, Dag::Node& _root);

}