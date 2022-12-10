#include <HMP/Meshing/Projection.hpp>

#include <cinolib/geometry/quad_utils.h>
#include <cinolib/meshes/polygonmesh.h>
#include <optional>
#include <algorithm>
#include <cinolib/export_surface.h>
#include <HMP/Meshing/defensiveAdvance.hpp>
#include <HMP/Meshing/fill.hpp>
#include <HMP/Meshing/Match.hpp>
#include <HMP/Meshing/smooth.hpp>

namespace HMP::Meshing::Projection
{

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
                for (I i{}; i < adjFids.size(); i++)
                {
                    for (const Match::SourceToTargetVid& match : _matches[toI(adjFids[i])])
                    {
                        weights.push_back(sourceVert.dist(match.pos));
                    }
                }
                invertAndNormalizeDistances(weights);
            }
            break;
            case EInvertMode::BarycentricCoords:
            {
                for (I i{}; i < adjFids.size(); i++)
                {
                    const Id adjFid{ adjFids[i] };
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
        invertAndNormalizeDistances(weights);
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
                const Real normDistance{ *normDistanceIt++ };
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

    std::optional<Vec> projectPathVert(const cinolib::AbstractPolygonMesh<>& _source, const Id _vid, const Options& _options)
    {
        return {};
    }

    std::vector<Vec> projectSurface(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Options& _options)
    {
        const std::vector<Match::TargetVidToSource>& matches{ Match::matchSurface(_source, _target) };
        const std::vector<std::vector<Match::SourceToTargetVid>>& invMatches{ Match::invertSurfaceMatches(_source, _target, matches) };
        std::vector<std::optional<Vec>> projected(toI(_source.num_verts()));
        for (Id vid{}; vid < _source.num_verts(); vid++)
        {
            projected[toI(vid)] = projectSurfaceVert(_source, _target, vid, invMatches, _options);
        }
        return fill(_source, projected, _options.unsetVertsDistWeightTweak);
    }

    std::vector<std::optional<Vec>> projectPath(const cinolib::AbstractPolygonMesh<>& _source, const cinolib::AbstractPolygonMesh<>& _target, const Path& _path, const Options& _options)
    {
        return {};
    }

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

    std::vector<Vec> project(const Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Point>& _pointFeats, const std::vector<Path>& _pathFeats, const Options& _options)
    {
        SurfaceExporter exporter{ _source };
        const std::vector<Point> surfPointFeats{ exporter.map(_pointFeats) };
        const std::vector<Path> surfPathFeats{ exporter.map(_pathFeats) };
        for (I i{}; i < _options.iterations; i++)
        {
            const bool lastIteration{ i + 1 == _options.iterations };
            if (i > 0)
            {
                exporter.surf.update_normals();
            }
            const std::vector<Vec> oldSurfVerts{ exporter.surf.vector_verts() };
            exporter.surf.vector_verts() = projectSurface(exporter.surf, _target, _options);
            if (_options.smoothSurface && !lastIteration)
            {
                smooth(exporter.surf);
            }
            // x = project path 
            // set x in exporter.surf
            // smooth paths only
            for (const Point& pointFeat : _pointFeats)
            {
                exporter.surf.vert(pointFeat.sourceVid) = _target.vert(pointFeat.targetVid);
            }
            if (_options.advancePercentile < 1.0 && !lastIteration)
            {
                defensiveAdvance(oldSurfVerts, exporter.surf.vector_verts(), exporter.surf.vector_verts(), _options.advancePercentile);
            }
            if (_options.smoothInternal)
            {
                exporter.applySurfToVol();
                smooth(exporter.vol);
            }
        }
        exporter.applySurfToVol();
        return exporter.vol.vector_verts();
    }

}
