#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>

namespace HMP::Meshing::Utils
{

	PolyVerts polyVertsFromFace(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid);
	PolyVerts polyVertsFromEdge(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid);

	Id closestFaceInPoly(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid);

	void addLeafs(Mesher& _mesher, Dag::Element& _root, bool _clear = true);

}