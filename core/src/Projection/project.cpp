#include <HMP/Projection/project.hpp>

#include <cinolib/geometry/quad_utils.h>
#include <cinolib/meshes/polygonmesh.h>
#include <optional>
#include <algorithm>
#include <HMP/Projection/defensiveAdvance.hpp>
#include <cpputils/collections/zip.hpp>
#include <HMP/Projection/fill.hpp>
#include <HMP/Projection/Match.hpp>
#include <HMP/Projection/smooth.hpp>
#include <array>
#include <cinolib/parallel_for.h>

namespace HMP::Projection
{

    static constexpr unsigned int c_minVertsForParallelFor{ 256 };

    // surface

    std::vector<Real> surfaceVertBaseWeights(const cinolib::AbstractPolygonMesh<>& _source, const Id _vid, const std::vector<std::vector<Match::SourceToTargetVid>>& _matches, const EBaseWeightMode _baseWeightMode)
    {
        const std::vector<Id>& adjFids{ _source.adj_v2p(_vid) };
        std::vector<Real> weights;
        {
            I weightCount{};
            for (const Id adjFid : adjFids)
            {
                weightCount += _matches[toI(adjFid)].size();
            }
            weights.reserve(weightCount);
        }
        switch (_baseWeightMode)
        {
            case EBaseWeightMode::Distance:
            {
                const Vec& sourceVert{ _source.vert(_vid) };
                for (const Id adjFid : adjFids)
                {
                    for (const Match::SourceToTargetVid& match : _matches[toI(adjFid)])
                    {
                        weights.push_back(sourceVert.dist(match.pos));
                    }
                }
                Utils::invertAndNormalizeDistances(weights);
            }
            break;
            case EBaseWeightMode::BarycentricCoords:
            {
                for (const Id adjFid : adjFids)
                {
                    const Id adjFO{ _source.poly_vert_offset(adjFid, _vid) };
                    const std::vector<Vec>& sourceFaceVerts{ _source.poly_verts(adjFid) };
                    for (const Match::SourceToTargetVid& match : _matches[toI(adjFid)])
                    {
                        cinolib::vec4<Real> sourceFaceWeights;
                        cinolib::quad_barycentric_coords(
                            sourceFaceVerts[0],
                            sourceFaceVerts[1],
                            sourceFaceVerts[2],
                            sourceFaceVerts[3],
                            match.pos,
                            sourceFaceWeights
                        );
                        weights.push_back(sourceFaceWeights[adjFO]);
                    }
                }
                Utils::normalizeWeights(weights);
            }
            break;
        }
        return weights;
    }

    std::vector<Real> surfaceVertNormDistances(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Id _vid, const std::vector<std::vector<Match::SourceToTargetVid>>& _matches)
    {
        const std::vector<Id>& adjFids{ _source.adj_v2p(_vid) };
        std::vector<Real> weights;
        {
            I weightCount{};
            for (const Id adjFid : adjFids)
            {
                if (!_matches[toI(adjFid)].empty())
                {
                    weightCount++;
                }
            }
            weights.reserve(weightCount);
        }
        const Vec& sourceVert{ _source.vert(_vid) };
        for (const Id adjFid : adjFids)
        {
            if (_matches[toI(adjFid)].empty())
            {
                continue;
            }
            const Vec& targetVert{ _target.vert(adjFid) };
            weights.push_back(targetVert.dist(sourceVert));
        }
        Utils::normalizeWeights(weights);
        return weights;
    }

    std::optional<Vec> projectSurfaceVert(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Id _vid, const std::vector<std::vector<Match::SourceToTargetVid>>& _matches, const Options& _options)
    {
        const std::vector<Id>& adjFids{ _source.adj_v2p(_vid) };
        const Vec& sourceVert{ _source.vert(_vid) };
        const std::vector<Real>& baseWeights{ surfaceVertBaseWeights(_source, _vid, _matches, _options.baseWeightMode) };
        const std::vector<Real>& normDistances{ surfaceVertNormDistances(_source, _target, _vid, _matches) };
        const Vec sourceNorm{ _source.vert_data(_vid).normal };
        Vec targetVertSum{};
        Vec dirSum{};
        Vec normDirSum{};
        Real dirLengthSum{};
        Real weightSum{};
        std::vector<Real>::const_iterator baseWeightIt{ baseWeights.begin() };
        std::vector<Real>::const_iterator normDistanceIt{ normDistances.begin() };
        for (I i{}; i < adjFids.size(); i++)
        {
            for (const Match::SourceToTargetVid& match : _matches[toI(adjFids[i])])
            {
                const Real baseWeight{ *baseWeightIt++ };
                if (_options.baseWeightTweak.shouldSkip(baseWeight))
                {
                    continue;
                }
                const Vec targetNorm{ _target.vert_data(match.targetVid).normal };
                const Real dot{ sourceNorm.dot(targetNorm) };
                if (_options.normalDotTweak.shouldSkip(dot))
                {
                    continue;
                }
                const Real normDistance{ *normDistanceIt };
                const Real distanceWeight{ std::pow(normDistance, _options.distanceWeightPower) * _options.distanceWeight + 1.0 };
                const Real weight{ _options.baseWeightTweak.apply(baseWeight) * _options.normalDotTweak.apply(dot) * distanceWeight };
                const Vec targetVert{ _target.vert(match.targetVid) };
                const Vec dir{ targetVert - sourceVert };
                weightSum += weight;
                targetVertSum += targetVert * weight;
                dirSum += dir * weight;
                normDirSum += dir.is_null() ? Vec{} : (dir.normalized() * weight);
                dirLengthSum += dir.norm() * weight;
            }
            if (!_matches[toI(adjFids[i])].empty())
            {
                normDistanceIt++;
            }
        }
        if (weightSum == 0)
        {
            return std::nullopt;
        }
        else
        {
            switch (_options.displaceMode)
            {
                case EDisplaceMode::VertAvg:
                    return targetVertSum / weightSum;
                    break;
                case EDisplaceMode::DirAvg:
                    return sourceVert + dirSum / weightSum;
                case EDisplaceMode::NormDirAvgAndDirAvg:
                    return sourceVert + normDirSum * ((dirSum / weightSum).norm() / weightSum);
                case EDisplaceMode::NormDirAvgAndDirNormAvg:
                    return sourceVert + normDirSum * dirLengthSum / weightSum / weightSum;
                default:
                    throw std::domain_error{ "unknown displace mode" };
            }
        }
    }

    std::vector<Vec> projectSurface(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Options& _options)
    {
        const std::vector<std::vector<Match::SourceToTargetVid>>& matches{ Match::matchSurfaceFid(_source, _target) };
        std::vector<std::optional<Vec>> projected(toI(_source.num_verts()));
        const auto func{ [&](const Id _vid) {
            projected[toI(_vid)] = projectSurfaceVert(_source, _target, _vid, matches, _options);
        } };
        cinolib::PARALLEL_FOR(0, _source.num_verts(), c_minVertsForParallelFor, func);
        return fill(_source, projected, _options.unsetVertsDistWeightTweak);
    }

    // path

    std::vector<Real> pathVertBaseWeights(const cinolib::AbstractPolygonMesh<>& _source, const Id _vid, const std::vector<Id>& _adjEids, const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>>& _matches, const EBaseWeightMode _baseWeightMode)
    {
        std::vector<Real> weights;
        {
            I weightCount{};
            for (const Id adjEid : _adjEids)
            {
                weightCount += _matches.at(adjEid).size();
            }
            weights.reserve(weightCount);
        }
        switch (_baseWeightMode)
        {
            case EBaseWeightMode::Distance:
            {
                const Vec& sourceVert{ _source.vert(_vid) };
                for (const Id adjEid : _adjEids)
                {
                    for (const Match::SourceToTargetVid& match : _matches.at(adjEid))
                    {
                        weights.push_back(sourceVert.dist(match.pos));
                    }
                }
                Utils::invertAndNormalizeDistances(weights);
            }
            break;
            case EBaseWeightMode::BarycentricCoords:
            {
                for (const Id adjEid : _adjEids)
                {
                    for (const Match::SourceToTargetVid& match : _matches.at(adjEid))
                    {
                        const Id otherVid{ _source.vert_opposite_to(adjEid, _vid) };
                        const Vec vert{ _source.vert(_vid) }, otherVert{ _source.vert(otherVid) };
                        const Vec adjEdgeDir{ otherVert - vert }, progressDir{ match.pos - vert };
                        const Real progress{ progressDir.dot(adjEdgeDir) / adjEdgeDir.dot(adjEdgeDir) };
                        weights.push_back(1.0 - progress);
                    }
                }
                Utils::normalizeWeights(weights);
            }
            break;
        }
        return weights;
    }

    std::optional<Vec> projectPathVert(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Id _vid, const std::vector<Id>& _adjEids, const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>>& _matches, const Options& _options)
    {
        const Vec& sourceVert{ _source.vert(_vid) };
        const std::vector<Real>& baseWeights{ pathVertBaseWeights(_source, _vid, _adjEids, _matches, _options.baseWeightMode) };
        const Vec sourceNorm{ _source.vert_data(_vid).normal };
        Vec targetVertSum{};
        Vec dirSum{};
        Vec normDirSum{};
        Real dirLengthSum{};
        Real weightSum{};
        std::vector<Real>::const_iterator baseWeightIt{ baseWeights.begin() };
        for (const Id adjEid : _adjEids)
        {
            for (const Match::SourceToTargetVid& match : _matches.at(adjEid))
            {
                const Real baseWeight{ *baseWeightIt++ };
                if (_options.baseWeightTweak.shouldSkip(baseWeight))
                {
                    continue;
                }
                const Real weight{ _options.baseWeightTweak.apply(baseWeight) };
                const Vec targetVert{ _target.vert(match.targetVid) };
                const Vec dir{ targetVert - sourceVert };
                weightSum += weight;
                targetVertSum += targetVert * weight;
                dirSum += dir * weight;
                normDirSum += dir.is_null() ? Vec{} : (dir.normalized() * weight);
                dirLengthSum += dir.norm() * weight;
            }
        }
        if (weightSum == 0)
        {
            return std::nullopt;
        }
        else
        {
            switch (_options.displaceMode)
            {
                case EDisplaceMode::VertAvg:
                    return targetVertSum / weightSum;
                    break;
                case EDisplaceMode::DirAvg:
                    return sourceVert + dirSum / weightSum;
                case EDisplaceMode::NormDirAvgAndDirAvg:
                    return sourceVert + normDirSum * ((dirSum / weightSum).norm() / weightSum);
                case EDisplaceMode::NormDirAvgAndDirNormAvg:
                    return sourceVert + normDirSum * dirLengthSum / weightSum / weightSum;
                default:
                    throw std::domain_error{ "unknown displace mode" };
            }
        }
    }

    std::vector<Vec> projectPath(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Id>& _sourceEidsPath, const std::vector<Id>& _sourceVidsPath, const std::vector<Id>& _targetVidsPath, const Options& _options)
    {
        const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>> matches{ Match::matchPathEid(_source, _target, _sourceEidsPath, _targetVidsPath) };
        std::vector<std::optional<Vec>> projected(_sourceVidsPath.size());
        const auto func{ [&](const Id _id) {
            const I i{ toI(_id) };
            const Id vid{ _sourceVidsPath[i] };
            const std::vector<Id> adjVids{ Utils::vidsPathAdjVids(_sourceVidsPath, i) };
            std::vector<Id> adjEids(adjVids.size());
            for (const auto& [adjVid, adjEid] : cpputils::collections::zip(adjVids, adjEids))
            {
                adjEid = _source.edge_id(vid, adjVid);
            }
            projected[i] = projectPathVert(_source, _target, vid, adjEids, matches, _options);
        } };
        cinolib::PARALLEL_FOR(0, toId(_sourceVidsPath.size()), c_minVertsForParallelFor, func);
        return fillPath(_source, projected, _options.unsetVertsDistWeightTweak, _sourceVidsPath);
    }

    // final projection

    std::vector<Vec> project(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options)
    {
        Utils::SurfaceExporter exporter{ _source };
        const std::vector<Id> onSurfVolVids{ exporter.onSurfVolVids() };
        const std::vector<Utils::Point> surfPointFeats{ Utils::toSurfFeats(_pointFeats, exporter) };
        const std::vector<Utils::EidsPath> surfEidsPathFeats{ Utils::toSurfFeats(_pathFeats, exporter) };
        const std::vector<Utils::VidsPath> surfVidsPathFeats{ Utils::eidsToVidsPaths(surfEidsPathFeats, exporter.surf, _target) };
        for (I i{}; i < _options.iterations; i++)
        {
            const bool lastIteration{ i + 1 == _options.iterations };
            if (i > 0 && _options.normalDotTweak.power() != 0.0)
            {
                exporter.surf.update_normals();
            }
            const std::vector<Vec> oldSurfVerts{ exporter.surf.vector_verts() };
            // surface
            exporter.surf.vector_verts() = projectSurface(exporter.surf, _target, _options);
            if (_options.smoothSurface && !lastIteration)
            {
                exporter.surf.vector_verts() = smooth(exporter.surf);
            }
            // paths
            for (const auto& [eidsPath, vidsPath] : cpputils::collections::zip(surfEidsPathFeats, surfVidsPathFeats))
            {
                Utils::setVerts(projectPath(exporter.surf, _target, eidsPath.sourceEids, vidsPath.sourceVids, vidsPath.targetVids, _options), exporter.surf.vector_verts(), vidsPath.sourceVids);
                if (_options.smoothSurface && !lastIteration)
                {
                    Utils::setVerts(smoothPath(exporter.surf, vidsPath.sourceVids), exporter.surf.vector_verts(), vidsPath.sourceVids);
                }
            }
            // points
            for (const Utils::Point& pointFeat : surfPointFeats)
            {
                exporter.surf.vert(pointFeat.sourceVid) = _target.vert(pointFeat.targetVid);
            }
            // advance
            if (_options.advancePercentile < 1.0 && !lastIteration)
            {
                defensiveAdvance(oldSurfVerts, exporter.surf.vector_verts(), exporter.surf.vector_verts(), _options.advancePercentile);
            }
            if (_options.smoothInternal)
            {
                exporter.applySurfToVol();
                exporter.vol.vector_verts() = smoothInternal(exporter.vol, onSurfVolVids);
            }
        }
        exporter.applySurfToVol();
        return exporter.vol.vector_verts();
    }

}
