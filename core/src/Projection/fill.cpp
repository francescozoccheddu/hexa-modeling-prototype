#include <HMP/Projection/fill.hpp>

#include <algorithm>
#include <limits>
#include <updatable_priority_queue.h>
#include <cpputils/collections/zip.hpp>

namespace HMP::Projection
{

    better_priority_queue::updatable_priority_queue<I, I> createQueue(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts)
    {
        better_priority_queue::updatable_priority_queue<I, I> queue{};
        for (I vi{}; vi < _newVerts.size(); vi++)
        {
            if (!_newVerts[vi])
            {
                I count{};
                for (const Id adjVid : _mesh.adj_v2v(toId(vi)))
                {
                    if (_newVerts[toI(adjVid)])
                    {
                        count++;
                    }
                }
                queue.push(vi, count);
            }
        }
        return queue;
    }

    better_priority_queue::updatable_priority_queue<I, I> createQueue(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, std::vector<Id> _vidsPath)
    {
        better_priority_queue::updatable_priority_queue<I, I> queue{};
        for (I i{}; i < _vidsPath.size(); i++)
        {
            const Id vid{ _vidsPath[i] };
            if (!_newVerts[toI(vid)])
            {
                I count{};
                for (const Id adjVid : Utils::vidsPathAdjVids(_vidsPath, i))
                {
                    if (_newVerts[toI(adjVid)])
                    {
                        count++;
                    }
                }
                queue.push(toI(vid), count);
            }
        }
        return queue;
    }

    std::vector<Vec> fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Utils::Tweak& _distWeightTweak, const std::optional<std::vector<Id>>& _vidsPath)
    {
        std::vector<Vec> out;
        fill(_mesh, _newVerts, _distWeightTweak, out, _vidsPath);
        return out;
    }

    void fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Utils::Tweak& _distWeightTweak, std::vector<Vec>& _out, const std::optional<std::vector<Id>>& _vidsPath)
    {
        better_priority_queue::updatable_priority_queue<I, I> skippedVisQueue{ _vidsPath
            ? createQueue(_mesh, _newVerts, *_vidsPath)
            : createQueue(_mesh, _newVerts)
        };
        std::vector<std::optional<Vec>> newVerts{ _newVerts };
        std::vector<Real> distances;
        distances.reserve(4);
        std::unordered_map<Id, I> vid2vidsI;
        if (_vidsPath)
        {
            const std::vector<Id>& vidsPath{ *_vidsPath };
            vid2vidsI.reserve(vidsPath.size());
            for (I i{}; i < vidsPath.size(); i++)
            {
                vid2vidsI.emplace(vidsPath[i], i);
            }
        }
        while (!skippedVisQueue.empty())
        {
            const I vi{ skippedVisQueue.pop_value(false).key };
            const Id vid{ toId(vi) };
            const Vec vert{ _mesh.vert(vid) };
            const std::vector<Id> adjVids{ _vidsPath
                ? Utils::vidsPathAdjVids(*_vidsPath, vid2vidsI.at(vid))
                : _mesh.adj_v2v(vid)
            };
            distances.clear();
            for (const Id adjVid : adjVids)
            {
                distances.push_back(vert.dist(_mesh.vert(adjVid)));
            }
            Utils::invertAndNormalizeDistances(distances);
            Real minOldDist{ std::numeric_limits<Real>::infinity() };
            Vec adjVertSum{};
            Real weightSum{};
            for (const auto [adjVid, baseWeight] : cpputils::collections::zip(adjVids, distances))
            {
                if (_distWeightTweak.shouldSkip(baseWeight))
                {
                    continue;
                }
                const Real weight{ _distWeightTweak.apply(weight) };
                const Vec adjVert{
                    newVerts[toI(adjVid)]
                    ? *newVerts[toI(adjVid)]
                    : _mesh.vert(adjVid)
                };
                weightSum += weight;
                adjVertSum += adjVert * weight;
            }
            if (weightSum != 0.0)
            {
                newVerts[vi] = adjVertSum / weightSum;
            }
            else
            {
                newVerts[vi] = vert;
            }
            for (const Id adjVid : adjVids)
            {
                if (!newVerts[toI(adjVid)])
                {
                    const I oldCount{ skippedVisQueue.get_priority(toI(adjVid)).second };
                    skippedVisQueue.update(toI(adjVid), oldCount + 1);
                }
            }
        }
        _out.resize(newVerts.size());
        for (const auto& [in, out] : cpputils::collections::zip(newVerts, _out))
        {
            out = *in;
        }
    }

}
