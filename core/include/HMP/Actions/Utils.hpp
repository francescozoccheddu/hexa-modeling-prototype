#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <vector>

namespace HMP::Actions::Utils
{

	Dag::Refine& prepareRefine(Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme);
	std::vector<PolyVerts> previewRefine(const Meshing::Mesher& _mesher, const Dag::Refine& _refine);
	void applyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine);
	void unapplyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine, bool _detach = true);

	Dag::Delete& prepareDelete();
	void applyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete);
	void unapplyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete, bool _detach = true);

	Dag::Extrude& prepareExtrude(Id _forwardFaceOffset, Id _upFaceOffset);
	void applyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _refine);
	void unapplyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _refine, bool _detach = true);

	void applyTree(Meshing::Mesher& _mesher, Dag::Node& _node);

}