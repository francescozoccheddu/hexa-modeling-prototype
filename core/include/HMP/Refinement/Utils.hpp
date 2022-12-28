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

namespace HMP::Refinement::Utils
{

    Dag::Refine& prepare(I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth = 1);
    void apply(Meshing::Mesher& _mesher, Dag::Refine& _refine);
    void applyRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine);

    class Sub3x3AdapterCandidate final
    {

    private:

        Dag::Element* m_element;
        std::optional < Refinement::EScheme > m_scheme;
        std::vector<EdgeVertIds> m_adjacentEdgeVertOffsets;
        std::vector<Id> m_adjacentFaceOffsets;
        Id m_forwardFaceOffset, m_upFaceOffset;

        void setup3x3Subdivide(const Meshing::Mesher& _mesher);

        void findRightAdapter(const Meshing::Mesher& _mesher);

    public:

        Sub3x3AdapterCandidate(Dag::Element& _element);

        Dag::Element& element() const;

        Refinement::EScheme scheme() const;

        void addAdjacency(const Meshing::Mesher& _mesher, const Dag::Element& _refined, bool _edge);

        Dag::Refine& prepareAdapter() const;

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