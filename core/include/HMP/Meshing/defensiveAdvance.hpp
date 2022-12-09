#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_mesh.h>
#include <vector>
#include <iterator>

namespace HMP::Meshing
{

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> defensiveAdvance(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Vec>& _newVerts, const double _percentile = 0.5);

    template<typename M, typename V, typename E, typename P, std::output_iterator<Id> TVidIt>
    std::vector<Vec> defensiveAdvance(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Vec>& _newVerts, const TVidIt _vidsBegin, const TVidIt _vidsEnd, const double _percentile = 0.5);

}

#define HMP_MESHING_DEFENSIVEADVANCE_IMPL
#include <HMP/Meshing/defensiveAdvance.tpp>
#undef HMP_MESHING_DEFENSIVEADVANCE_IMPL