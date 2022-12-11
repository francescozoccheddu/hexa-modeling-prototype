#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <vector>
#include <unordered_set>
#include <optional>
#include <HMP/Projection/Utils.hpp>

namespace HMP::Projection
{

    void fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Utils::Tweak& _distWeightTweak, std::vector<Vec>& _out, const std::optional<std::vector<Id>>& _vidsPath = std::nullopt);

    std::vector<Vec> fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Utils::Tweak& _distWeightTweak, const std::optional<std::vector<Id>>& _vidsPath = std::nullopt);

}
