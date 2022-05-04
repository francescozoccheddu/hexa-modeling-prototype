#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/grid.hpp>

namespace HMP::Meshing::Utils
{

	PolyVerts polyVertsFromFace(const Grid::Mesh& _mesh, Id _pid, Id _fid);
	PolyVerts polyVertsFromEdge(const Grid::Mesh& _mesh, Id _pid, Id _eid);

}