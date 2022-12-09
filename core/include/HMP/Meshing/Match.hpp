#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <vector>
#include <unordered_map>

namespace HMP::Meshing::Match
{

    struct TargetVidToSource final
    {
        Vec pos;
        Id sourceId;
    };

    struct SourceToTargetVid final
    {
        Vec pos;
        Id targetVid;
    };

    std::vector<TargetVidToSource> matchSurface(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target);

    std::vector<TargetVidToSource> matchPath(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Id>& _sourceEids, const std::vector<Id>& _targetVids);

    std::vector<std::vector<SourceToTargetVid>> invertSurfaceMatches(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<TargetVidToSource>& _matches);

    std::unordered_map<Id, std::vector<SourceToTargetVid>> invertPathMatches(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Id>& _sourceEids, const std::vector<Id>& _targetVids, const std::vector<TargetVidToSource>& _matches);

}
