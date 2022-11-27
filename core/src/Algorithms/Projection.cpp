#include <HMP/Algorithms/Projection.hpp>

#include <stdexcept>
#include <cinolib/meshes/hexmesh.h>
#include <cinolib/meshes/polygonmesh.h>
#include <cinolib/meshes/quadmesh.h>
#include <vector>
#include <cinolib/octree.h>
#include <cinolib/geometry/quad_utils.h>
#include <optional>
#include <algorithm>
#include <limits>
#include <updatable_priority_queue.h>
#include <cinolib/parallel_for.h>
#include <cinolib/export_surface.h>

namespace HMP::Algorithms::Projection
{

    constexpr Id i2id(std::size_t _i) { return static_cast<Id>(_i); }
    constexpr std::size_t id2i(Id _id) { return static_cast<std::size_t>(_id); }

    struct TargetToSourceMatch final
    {
        Vec pos;
        Id sourceFid;
    };

    struct SourceToTargetMatch final
    {
        Vec dir;
        Id targetVid;
        Real weight;
    };

    Tweak::Tweak(Real _min, Real _power) : m_min{ _min }, m_power{ _power }
    {
        if (_power < 0.0 || _power > 10.0)
        {
            throw std::logic_error{ "power out of range" };
        }
    }

    bool Tweak::shouldSkip(Real _value) const
    {
        return _value < m_min;
    }

    Real Tweak::apply(Real _value) const
    {
        return std::pow((_value - m_min) / (1.0 - m_min), m_power);
    }

    std::vector<TargetToSourceMatch> matchTargetToSource(const cinolib::Polygonmesh<>& _source, const cinolib::Polygonmesh<>& _target)
    {
        std::vector<TargetToSourceMatch> matches(id2i(_target.num_verts()));
        cinolib::Octree sourceOctree{};
        sourceOctree.build_from_mesh_polys(_source);
        cinolib::PARALLEL_FOR(0, _target.num_verts(), 64, [&_target, &sourceOctree, &matches](Id _targetVid) {
            const Vec& targetVert{ _target.vert(_targetVid) };
            Vec sourceVert;
            Id sourceFid;
            Real dist;
            sourceOctree.closest_point(targetVert, sourceFid, sourceVert, dist);
            matches[id2i(_targetVid)] = {
                .pos = sourceVert,
                .sourceFid = sourceFid
            };
        });
        return matches;
    }

    std::vector<std::vector<SourceToTargetMatch>> invertTargetToSourceMatches(const cinolib::Polygonmesh<>& _source, const cinolib::Polygonmesh<>& _target, const std::vector<TargetToSourceMatch>& _matches, EInvertMode _mode)
    {
        std::vector<std::vector<SourceToTargetMatch>> invMatches(id2i(_source.num_verts()));
        for (std::size_t targetVi{}; targetVi < _matches.size(); targetVi++)
        {
            const Id targetVid{ i2id(targetVi) };
            const Vec& targetVert{ _target.vert(targetVid) };
            const TargetToSourceMatch& match{ _matches[targetVi] };
            SourceToTargetMatch invMatch;
            cinolib::vec4<Real> sourceFaceWeights{};
            if (_mode == EInvertMode::BarycentricCoords)
            {
                const std::vector<Vec> sourceFaceVerts{ _source.poly_verts(match.sourceFid) };
                cinolib::quad_barycentric_coords(
                    sourceFaceVerts[0],
                    sourceFaceVerts[1],
                    sourceFaceVerts[2],
                    sourceFaceVerts[3],
                    match.pos,
                    sourceFaceWeights
                );
            }
            for (const Id sourceVid : _source.adj_p2v(match.sourceFid))
            {
                SourceToTargetMatch invMatch;
                invMatch.dir = targetVert - match.pos;
                invMatch.targetVid = targetVid;
                switch (_mode)
                {
                    case EInvertMode::BarycentricCoords:
                    {
                        const Id sourceVertOffset{ _source.poly_vert_offset(match.sourceFid, sourceVid) };
                        invMatch.weight = sourceFaceWeights[sourceVertOffset];
                        break;
                    }
                    case EInvertMode::Distance:
                    {
                        const Real distance{ _source.vert(sourceVid).dist(targetVert) };
                        invMatch.weight = distance;
                        break;
                    }
                }
                invMatches[id2i(sourceVid)].push_back(invMatch);
            }
        }

        if (_mode == EInvertMode::Distance)
        {
            for (std::vector<SourceToTargetMatch>& vidMatches : invMatches)
            {
                if (vidMatches.empty())
                {
                    continue;
                }
                Real minDist{ std::numeric_limits<Real>::infinity() };
                for (const SourceToTargetMatch& match : vidMatches)
                {
                    if (match.weight < minDist)
                    {
                        minDist = match.weight;
                    }
                }
                if (minDist == 0)
                {
                    vidMatches.erase(
                        std::remove_if(
                            vidMatches.begin(),
                            vidMatches.end(),
                            [](const SourceToTargetMatch& _match) { return _match.weight != 0; }
                        ),
                        vidMatches.end()
                    );
                }
                else
                {
                    for (SourceToTargetMatch& match : vidMatches)
                    {
                        match.weight = minDist / match.weight;
                    }
                }
            }
        }

        return invMatches;
    }

    std::vector<std::optional<Vec>> displace(const cinolib::Polygonmesh<>& _source, const cinolib::Polygonmesh<>& _target, const std::vector<std::vector<SourceToTargetMatch>>& _matches, EDisplaceMode _mode, const Tweak& _weightTweak, const Tweak& _normDotTweak)
    {
        std::vector<std::optional<Vec>> newSourceVerts(id2i(_source.num_verts()), std::nullopt);

        for (Id sourceVid{}; sourceVid < _source.num_verts(); sourceVid++)
        {
            const Vec sourceNorm{ _source.vert_data(sourceVid).normal };
            Real maxWeight{};
            for (const SourceToTargetMatch& match : _matches[id2i(sourceVid)])
            {
                maxWeight = std::max(match.weight, maxWeight);
            }
            if (maxWeight == 0.0)
            {
                maxWeight = 1.0;
            }
            Vec targetVertSum{};
            Vec dirSum{};
            Vec normDirSum{};
            Real dirLengthSum{};
            Real weightSum{};
            for (const SourceToTargetMatch& match : _matches[id2i(sourceVid)])
            {
                const Real normMatchWeight{ match.weight / maxWeight };
                if (_weightTweak.shouldSkip(normMatchWeight))
                {
                    continue;
                }
                const Vec targetNorm{ _target.vert_data(match.targetVid).normal };
                const Real dot{ sourceNorm.dot(targetNorm) };
                if (_normDotTweak.shouldSkip(dot))
                {
                    continue;
                }
                const Real weight{ _weightTweak.apply(normMatchWeight) * _normDotTweak.apply(dot) };
                weightSum += weight;
                targetVertSum += _target.vert(match.targetVid) * weight;
                dirSum += match.dir * weight;
                normDirSum += match.dir.normalized() * weight;
                dirLengthSum += match.dir.norm() * weight;
            }
            if (weightSum != 0)
            {
                std::optional<Vec>& vert{ newSourceVerts[id2i(sourceVid)] };
                switch (_mode)
                {
                    case EDisplaceMode::VertAvg:
                        vert = targetVertSum / weightSum;
                        break;
                    case EDisplaceMode::DirAvg:
                        vert = _source.vert(sourceVid) + dirSum / weightSum;
                        break;
                    case EDisplaceMode::NormDirAvgAndDirAvg:
                        vert = _source.vert(sourceVid) + normDirSum * ((dirSum / weightSum).norm() / weightSum);
                        break;
                    case EDisplaceMode::NormDirAvgAndDirNormAvg:
                        vert = _source.vert(sourceVid) + normDirSum * dirLengthSum / weightSum / weightSum;
                        break;
                }
            }
        }
        return newSourceVerts;
    }

    std::vector<Vec> processSkippedVerts(const cinolib::Polygonmesh<>& _source, const std::vector<std::optional<Vec>>& _newSourceVerts, const Tweak& _distWeightTweak)
    {
        better_priority_queue::updatable_priority_queue<std::size_t, std::size_t> skippedVisQueue{};
        std::vector<std::optional<Vec>> newVerts{ _newSourceVerts };

        for (std::size_t vi{}; vi < newVerts.size(); vi++)
        {
            if (!newVerts[vi])
            {
                std::size_t count{};
                for (const Id adjVid : _source.adj_v2v(i2id(vi)))
                {
                    if (!newVerts[id2i(adjVid)])
                    {
                        count++;
                    }
                }
                skippedVisQueue.push(vi, std::numeric_limits<std::size_t>::max() - count);
            }
        }

        while (!skippedVisQueue.empty())
        {
            const std::size_t vi{ skippedVisQueue.pop_value(false).key };
            const Id vid{ i2id(vi) };
            const Vec vert{ _source.vert(vid) };
            Real minOldDist{ std::numeric_limits<Real>::infinity() };
            for (const Id adjVid : _source.adj_v2v(vid))
            {
                minOldDist = std::min(minOldDist, vert.dist(_source.vert(adjVid)));
            }
            Vec adjVertSum{};
            Real weightSum{};
            for (const Id adjVid : _source.adj_v2v(vid))
            {
                const Real oldDist{ vert.dist(_source.vert(adjVid)) };
                if (minOldDist == 0 && oldDist != 0)
                {
                    continue;
                }
                const Real normWeight{ minOldDist == 0 ? 1 : (minOldDist / oldDist) };
                if (_distWeightTweak.shouldSkip(normWeight))
                {
                    continue;
                }
                const Real weight{ _distWeightTweak.apply(normWeight) };
                const Vec adjVert{
                    newVerts[id2i(adjVid)]
                    ? *newVerts[id2i(adjVid)]
                    : _source.vert(adjVid)
                };
                weightSum += weight;
                adjVertSum += adjVert * weight;
            }
            newVerts[vi] = adjVertSum / weightSum;
            for (const Id adjVid : _source.adj_v2v(vid))
            {
                if (!newVerts[id2i(adjVid)])
                {
                    const std::size_t oldCount{ skippedVisQueue.get_priority(id2i(adjVid)).second };
                    skippedVisQueue.update(id2i(adjVid), oldCount + 1);
                }
            }
        }

        std::vector<Vec> newActualVerts{};
        newActualVerts.reserve(newVerts.size());
        std::transform(newVerts.begin(), newVerts.end(), std::back_inserter(newActualVerts), [](const std::optional<Vec>& _vec) { return *_vec; });
        return newActualVerts;
    }

    std::vector<Vec> defensiveAdvance(const cinolib::Polygonmesh<>& _source, const std::vector<Vec>& _newSourceVerts, double _percentile)
    {
        std::vector<Vec> newVerts(_newSourceVerts.size());
        Real maxLength{};
        {
            std::vector<Real> lengths(_newSourceVerts.size());
            for (std::size_t vi{}; vi < _newSourceVerts.size(); vi++)
            {
                lengths[vi] = (_newSourceVerts[vi] - _source.vert(i2id(vi))).norm();
            }
            std::sort(lengths.begin(), lengths.end());
            std::size_t medianI{ static_cast<std::size_t>(std::round(static_cast<double>(lengths.size() - 1) * _percentile)) };
            if (medianI <= lengths.size())
            {
                maxLength = lengths[medianI];
            }
            else
            {
                maxLength = std::numeric_limits<Real>::infinity();
            }
        }
        for (std::size_t vi{}; vi < _newSourceVerts.size(); vi++)
        {
            const Vec sourceVert{ _source.vert(i2id(vi)) };
            const Vec offset{ _newSourceVerts[vi] - sourceVert };
            const Vec clampedOffset{ offset.norm() <= maxLength ? offset : (offset.normalized() * maxLength) };
            newVerts[vi] = sourceVert + clampedOffset;
        }
        return newVerts;
    }

    std::vector<Vec> smooth(const cinolib::Polygonmesh<>& _source, const std::vector<Vec>& _newSourceVerts)
    {
        std::vector<Vec> newVerts(_newSourceVerts.size());
        for (std::size_t vi{}; vi < _newSourceVerts.size(); vi++)
        {
            Vec vertSum{};
            Real weightSum{};
            for (const Id& adjVid : _source.adj_v2v(i2id(vi)))
            {
                const Real weight = 1.0;
                vertSum += _newSourceVerts[i2id(adjVid)] * weight;
                weightSum += weight;
            }
            newVerts[vi] = vertSum / weightSum;
        }
        return newVerts;
    }

    std::vector<Vec> project(const cinolib::Polygonmesh<>& _source, const cinolib::Polygonmesh<>& _target, const Options& _options)
    {
        cinolib::Polygonmesh<> source{ _source };
        for (std::size_t i{}; i < _options.iterations; i++)
        {
            const bool lastIteration{ i + 1 == _options.iterations };
            if (i > 0)
            {
                source.update_normals();
            }
            const std::vector<TargetToSourceMatch> matches{ matchTargetToSource(source, _target) };
            const std::vector<std::vector<SourceToTargetMatch>> invMatches{ invertTargetToSourceMatches(source, _target, matches, _options.invertMode) };
            const std::vector<std::optional<Vec>> displaceVerts{ displace(source, _target, invMatches, _options.displaceMode, _options.weightTweak, _options.normalDotTweak) };
            const std::vector<Vec> displaceAndFillVerts{ processSkippedVerts(source, displaceVerts, _options.unsetVertsDistWeightTweak) };
            const std::vector<Vec> displaceAndFillAndAdvanceVerts{ defensiveAdvance(source, displaceAndFillVerts, lastIteration ? 1.0 : _options.advancePercentile) };
            const std::vector<Vec> displaceAndFillAndAdvanceAndSmoothVerts{ (lastIteration && _options.smooth) ? smooth(source, displaceAndFillAndAdvanceVerts) : displaceAndFillAndAdvanceVerts };
            for (std::size_t vi{}; vi < displaceAndFillAndAdvanceAndSmoothVerts.size(); vi++)
            {
                source.vert(i2id(vi)) = displaceAndFillAndAdvanceAndSmoothVerts[vi];
            }
        }
        std::vector<Vec> verts(id2i(source.num_verts()));
        for (Id vid{}; vid < source.num_verts(); vid++)
        {
            verts[id2i(vid)] = source.vert(vid);
        }
        return verts;
    }

}
