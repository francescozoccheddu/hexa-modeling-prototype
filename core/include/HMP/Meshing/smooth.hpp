#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_mesh.h>
#include <vector>
#include <unordered_set>

namespace HMP::Meshing
{

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh);

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::unordered_set<Id>& _vids);

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Vec>& _verts);

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Vec>& _verts, const std::unordered_set<Id>& _vids);

}

#define HMP_MESHING_SMOOTH_IMPL
#include <HMP/Meshing/smooth.tpp>
#undef HMP_MESHING_SMOOTH_IMPL