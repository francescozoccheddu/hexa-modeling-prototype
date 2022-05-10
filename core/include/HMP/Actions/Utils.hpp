#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Extrude.hpp>

namespace HMP::Actions::Utils
{

	Dag::Refine& prepareRefine(Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme);
	void applyRefine(Meshing::Mesher& _mesher, Dag::Element& _element, Dag::Refine& _refine);
	void unapplyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine);
	
	Dag::Delete& prepareDelete();
	void applyDelete(Meshing::Mesher& _mesher, Dag::Element& _element, Dag::Delete& _delete);
	void unapplyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete);

	Dag::Extrude& prepareExtrude(Id _forwardFaceOffset, Id _upFaceOffset);
	void applyExtrude(Meshing::Mesher& _mesher, Dag::Element& _element, Dag::Extrude& _refine);
	void unapplyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _refine);

}