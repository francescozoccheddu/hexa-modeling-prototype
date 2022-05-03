#pragma once

#include <HMP/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>

namespace HMP::Meshing::Utils
{

	PolyVerts polyVertsFromFace(const Mesher::Mesh& _mesh, Id _pid, Id _fid);
	PolyVerts polyVertsFromEdge(const Mesher::Mesh& _mesh, Id _pid, Id _eid);

}