#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/refinementSchemes.hpp>
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

	Dag::Refine& prepareRefine(Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme, I _depth = 1);
	std::vector<PolyVerts> previewRefine(const Meshing::Mesher& _mesher, const Dag::Refine& _refine);
	void applyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine);
	void applyRefineRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine);
	void unapplyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine, bool _detach = true);
	void unapplyRefineRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine, bool _detach = true);

	Dag::Delete& prepareDelete();
	void applyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete);
	void unapplyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete, bool _detach = true);

	Dag::Extrude& prepareExtrude(Id _firstUpFaceOffset, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets);
	void applyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _refine);
	void unapplyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _refine, bool _detach = true);

	void applyTree(Meshing::Mesher& _mesher, Dag::Node& _node);

	class Sub3x3AdapterCandidate final
	{

	private:

		Dag::Element* m_element;
		std::optional < Meshing::ERefinementScheme > m_scheme;
		std::vector<EdgeVertIds> m_adjacentEdgeVertOffsets;
		std::vector<Id> m_adjacentFaceOffsets;
		Id m_forwardFaceOffset, m_upFaceOffset;

		void setup3x3Subdivide(const Meshing::Mesher& _mesher);

		void findRightAdapter(const Meshing::Mesher& _mesher);

	public:

		Sub3x3AdapterCandidate(Dag::Element& _element);

		Dag::Element& element() const;

		Meshing::ERefinementScheme scheme() const;

		void addAdjacency(const Meshing::Mesher& _mesher, const Dag::Element& _refined, bool _edge);

		Dag::Refine& prepareAdapter(const Meshing::Mesher& _mesher) const;

	};

	class Sub3x3AdapterCandidateSet final
	{

	private:

		using Map = std::unordered_map<Dag::Element*, Sub3x3AdapterCandidate>;

		Map m_sub3x3Map{}; // candidates that will be refined as a new Subdivide3x3 (they need to be processed first)
		Map m_nonSub3x3Map{}; // other candidates (they must be processed only after)

		void addAdjacency(const Meshing::Mesher& _mesher, Dag::Element& _candidate, const Dag::Element& _refined, bool _edge);

	public:

		void addAdjacency(Meshing::Mesher& _mesher, Dag::Refine& _refine);

		Sub3x3AdapterCandidate pop();

		bool empty() const;

	};

}