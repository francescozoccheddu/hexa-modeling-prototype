#include <HMP/Meshing/Projection.hpp>

#include <cinolib/geometry/quad_utils.h>
#include <cinolib/meshes/polygonmesh.h>
#include <optional>
#include <algorithm>
#include <cinolib/export_surface.h>
#include <HMP/Meshing/defensiveAdvance.hpp>
#include <cpputils/collections/zip.hpp>
#include <HMP/Meshing/fill.hpp>
#include <HMP/Meshing/Match.hpp>
#include <HMP/Meshing/smooth.hpp>
#include <array>

namespace HMP::Meshing::Projection
{

    // utils

    Tweak::Tweak(Real _min, Real _power): m_min{ _min }, m_power{ _power }
    {
        if (_power < 0.0 || _power > 10.0)
        {
            throw std::logic_error{ "power out of range" };
        }
    }

    Real Tweak::min() const
    {
        return m_min;
    }

    Real Tweak::power() const
    {
        return m_power;
    }

    bool Tweak::shouldSkip(Real _value) const
    {
        return _value < m_min;
    }

    Real Tweak::apply(Real _value) const
    {
        return std::pow((_value - m_min) / (1.0 - m_min), m_power);
    }

    void normalizeWeights(std::vector<Real>& _weights)
    {
        if (_weights.empty())
        {
            return;
        }
        const Real maxWeight{ *std::max_element(_weights.begin(), _weights.end()) };
        if (maxWeight != 0.0)
        {
            for (Real& w : _weights)
            {
                w /= maxWeight;
            }
        }
    }

    void invertAndNormalizeDistances(std::vector<Real>& _distances)
    {
        if (_distances.empty())
        {
            return;
        }
        const Real minDist{ *std::min_element(_distances.begin(), _distances.end()) };
        if (minDist != 0.0)
        {
            for (Real& d : _distances)
            {
                d = minDist / d;
            }
        }
        else
        {
            for (Real& d : _distances)
            {
                d = d == 0.0 ? 1.0 : 0.0;
            }
        }
    }

    // surface

    std::vector<Real> surfaceVertBaseWeights(const cinolib::AbstractPolygonMesh<>& _source, const Id _vid, const std::vector<std::vector<Match::SourceToTargetVid>>& _matches, const EInvertMode _invertMode)
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
        switch (_invertMode)
        {
            case EInvertMode::Distance:
            {
                const Vec& sourceVert{ _source.vert(_vid) };
                for (const Id adjFid : adjFids)
                {
                    for (const Match::SourceToTargetVid& match : _matches[toI(adjFid)])
                    {
                        weights.push_back(sourceVert.dist(match.pos));
                    }
                }
                invertAndNormalizeDistances(weights);
            }
            break;
            case EInvertMode::BarycentricCoords:
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
                normalizeWeights(weights);
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
        normalizeWeights(weights);
        return weights;
    }

    std::optional<Vec> projectSurfaceVert(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Id _vid, const std::vector<std::vector<Match::SourceToTargetVid>>& _matches, const Options& _options)
    {
        const std::vector<Id>& adjFids{ _source.adj_v2p(_vid) };
        const Vec& sourceVert{ _source.vert(_vid) };
        const std::vector<Real>& baseWeights{ surfaceVertBaseWeights(_source, _vid, _matches, _options.invertMode) };
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
        for (Id vid{}; vid < _source.num_verts(); vid++)
        {
            projected[toI(vid)] = projectSurfaceVert(_source, _target, vid, matches, _options);
        }
        return fill(_source, projected, _options.unsetVertsDistWeightTweak);
    }

    // path

    std::vector<Real> pathVertBaseWeights(const cinolib::AbstractPolygonMesh<>& _source, const Id _vid, const std::vector<Id>& _adjEids, const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>>& _matches, const EInvertMode _invertMode)
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
        switch (_invertMode)
        {
            case EInvertMode::Distance:
            {
                const Vec& sourceVert{ _source.vert(_vid) };
                for (const Id adjEid : _adjEids)
                {
                    for (const Match::SourceToTargetVid& match : _matches.at(adjEid))
                    {
                        weights.push_back(sourceVert.dist(match.pos));
                    }
                }
                invertAndNormalizeDistances(weights);
            }
            break;
            case EInvertMode::BarycentricCoords:
            {
                for (const Id adjEid : _adjEids)
                {
                    for (const Match::SourceToTargetVid& match : _matches.at(adjEid))
                    {
                        const Id vid0{ _source.edge_vert_id(adjEid, 0) }, vid1{ _source.edge_vert_id(adjEid, 1) };
                        const Vec vert0{ _source.vert(vid0) }, vert1{ _source.vert(vid1) };
                        const Vec adjEdgeDir{ vert1 - vert0 }, progressDir{ match.pos - vert0 };
                        const Real progress{ progressDir.dot(adjEdgeDir) / adjEdgeDir.dot(adjEdgeDir) };
                        weights.push_back(_vid == vid1 ? progress : (1.0 - progress));
                    }
                }
                normalizeWeights(weights);
            }
            break;
        }
        return weights;
    }

    std::optional<Vec> projectPathVert(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Id _vid, const std::vector<Id>& _adjEids, const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>>& _matches, const Options& _options)
    {
        const Vec& sourceVert{ _source.vert(_vid) };
        const std::vector<Real>& baseWeights{ pathVertBaseWeights(_source, _vid, _adjEids, _matches, _options.invertMode) };
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

    std::vector<Vec> projectPath(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::unordered_set<Id>& _sourceEids, const std::unordered_set<Id>& _sourceVids, const std::vector<Id>& _targetVids, const Options& _options)
    {
        const std::unordered_map<Id, std::vector<Match::SourceToTargetVid>>& matches{ Match::matchPathEid(_source, _target, _sourceEids, _targetVids) };
        std::vector<std::optional<Vec>> projected;
        projected.reserve(_sourceVids.size());
        std::vector<Id> adjEids;
        adjEids.reserve(2);
        for (const Id vid : _sourceVids)
        {
            adjEids.clear();
            for (const Id adjEid : _source.adj_v2e(vid))
            {
                if (_sourceEids.contains(adjEid))
                {
                    adjEids.push_back(adjEid);
                }
            }
            projected.push_back(projectPathVert(_source, _target, vid, adjEids, matches, _options));
        }
        return fill(_source, projected, _options.unsetVertsDistWeightTweak, _sourceVids, _sourceEids);
    }

    // final projection

    class SurfaceExporter final
    {

    private:

        std::unordered_map<Id, Id> m_surf2vol, m_vol2surf;

    public:

        cinolib::Polygonmesh<> surf;
        Mesher::Mesh vol;

        SurfaceExporter(const Mesher::Mesh& _mesh): vol{ _mesh }
        {
            cinolib::export_surface(_mesh, surf, m_vol2surf, m_surf2vol);
        }

        void applySurfToVol()
        {
            for (Id surfVid{}; surfVid < surf.num_verts(); surfVid++)
            {
                vol.vert(m_surf2vol.at(surfVid)) = surf.vert(surfVid);
            }
        }

        void applyVolToSurf()
        {
            for (Id volVid{}; volVid < vol.num_verts(); volVid++)
            {
                surf.vert(m_vol2surf.at(volVid)) = vol.vert(volVid);
            }
        }

        std::vector<Point> map(const std::vector<Point>& _feats)  const
        {
            std::vector<Point> newFeats{ _feats };
            for (Point& feat : newFeats)
            {
                feat.sourceVid = m_vol2surf.at(feat.sourceVid);
            }
            return newFeats;
        }

        std::vector<Path> map(const std::vector<Path>& _feats)  const
        {
            std::vector<Path> newFeats{ _feats };
            for (Path& feat : newFeats)
            {
                for (Id& eid : feat.sourceEids)
                {
                    eid = surf.edge_id(
                        m_vol2surf.at(vol.edge_vert_id(eid, 0)),
                        m_vol2surf.at(vol.edge_vert_id(eid, 1))
                    );
                }
            }
            return newFeats;
        }

    };

    const std::unordered_set<Id> extractVids(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<Id>& _eids)
    {
        std::unordered_set<Id> set{};
        set.reserve(_eids.size() + 1);
        for (const Id eid : _eids)
        {
            set.insert(_mesh.edge_vert_id(eid, 0));
            set.insert(_mesh.edge_vert_id(eid, 1));
        }
        return set;
    }

    const std::vector<std::unordered_set<Id>> extractSourceEids(const cinolib::AbstractPolygonMesh<>& _source, const std::vector<Path>& _pathFeats)
    {
        std::vector<std::unordered_set<Id>> featEids(_pathFeats.size());
        for (const auto& [eids, path] : cpputils::collections::zip(featEids, _pathFeats))
        {
            eids = std::unordered_set<Id>{ path.sourceEids.begin(), path.sourceEids.end() };
        }
        return featEids;
    }

    const std::vector<std::unordered_set<Id>> extractSourceVids(const cinolib::AbstractPolygonMesh<>& _source, const std::vector<Path>& _pathFeats)
    {
        std::vector<std::unordered_set<Id>> featVids(_pathFeats.size());
        for (const auto& [vids, path] : cpputils::collections::zip(featVids, _pathFeats))
        {
            vids = extractVids(_source, path.sourceEids);
        }
        return featVids;
    }

    const std::vector<std::vector<Id>> extractTargetVids(const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Path>& _pathFeats)
    {
        std::vector<std::vector<Id>> featVids(_pathFeats.size());
        for (const auto& [vids, path] : cpputils::collections::zip(featVids, _pathFeats))
        {
            const std::unordered_set<Id>& vidsSet{ extractVids(_target, path.targetEids) };
            vids = { vidsSet.begin(), vidsSet.end() };
        }
        return featVids;
    }

    void setVerts(const std::vector<Vec>& _from, std::vector<Vec>& _to, const std::unordered_set<Id>& _vids)
    {
        for (const auto& [vid, from] : cpputils::collections::zip(_vids, _from))
        {
            _to[toI(vid)] = from;
        }
    }

    std::vector<Vec> project(const Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Point>& _pointFeats, const std::vector<Path>& _pathFeats, const Options& _options)
    {
        SurfaceExporter exporter{ _source };
        const std::vector<Point> surfPointFeats{ exporter.map(_pointFeats) };
        const std::vector<Path> surfPathFeats{ exporter.map(_pathFeats) };
        const std::vector<std::unordered_set<Id>> sourceSurfPathVids{ extractSourceVids(exporter.surf, surfPathFeats) };
        const std::vector<std::unordered_set<Id>> sourceSurfPathEids{ extractSourceEids(exporter.surf, surfPathFeats) };
        const std::vector<std::vector<Id>> targetPathVids{ extractTargetVids(_target, surfPathFeats) };
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
            for (const auto& [sourceEids, sourceVids, targetVids] : cpputils::collections::zip(sourceSurfPathEids, sourceSurfPathVids, targetPathVids))
            {
                setVerts(projectPath(exporter.surf, _target, sourceEids, sourceVids, targetVids, _options), exporter.surf.vector_verts(), sourceVids);
                if (_options.smoothSurface && !lastIteration)
                {
                    setVerts(smooth(exporter.surf, sourceVids, sourceEids), exporter.surf.vector_verts(), sourceVids);
                }
            }
            // points
            for (const Point& pointFeat : surfPointFeats)
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
                exporter.vol.vector_verts() = smooth(exporter.vol);
            }
        }
        exporter.applySurfToVol();
        return exporter.vol.vector_verts();
    }

}
