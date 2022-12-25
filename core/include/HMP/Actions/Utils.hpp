#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Refinement/Schemes.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <vector>
#include <optional>
#include <unordered_set>
#include <cstddef>

namespace HMP::Actions::Utils
{

	PolyVerts shapeExtrude(const Meshing::Mesher::Mesh& _mesh, const cpputils::collections::FixedVector<Id, 3>& _pids, const cpputils::collections::FixedVector< Id, 3>& _fids, Id _firstVid, bool _clockwise);
	PolyVerts shapeExtrude(const Meshing::Mesher& _mesher, const cpputils::collections::FixedVector<const Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Id _vertOffset, bool _clockwise);
	PolyVerts shapeExtrude(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude);
	Dag::Extrude& prepareExtrude(Id _vertOffset, bool _clockwise, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets);
	void applyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _extrude);
	void unapplyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _extrude, bool _detach = true);

}