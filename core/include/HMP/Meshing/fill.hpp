#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <vector>
#include <unordered_set>
#include <optional>
#include <HMP/Meshing/Projection.hpp>

namespace HMP::Meshing
{

    std::vector<Vec> fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Projection::Tweak& _distWeightTweak, const std::optional<std::unordered_set<Id>>& _vids = std::nullopt, const std::optional<std::unordered_set<Id>>& _eids = std::nullopt);

}
