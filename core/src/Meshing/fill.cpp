#include <HMP/Meshing/fill.hpp>

#include <algorithm>
#include <limits>
#include <updatable_priority_queue.h>
#include <cpputils/collections/zip.hpp>

namespace HMP::Meshing
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
                queue.push(vi, std::numeric_limits<I>::max() - count);
            }
        }
        return queue;
    }

    better_priority_queue::updatable_priority_queue<I, I> createQueue(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, std::unordered_set<Id> _vids)
    {
        better_priority_queue::updatable_priority_queue<I, I> queue{};
        for (const Id vid : _vids)
        {
            if (!_newVerts[toI(vid)])
            {
                I count{};
                for (const Id adjVid : _mesh.adj_v2v(vid))
                {
                    if (_newVerts[toI(adjVid)] && _vids.contains(adjVid))
                    {
                        count++;
                    }
                }
                queue.push(toI(vid), std::numeric_limits<I>::max() - count);
            }
        }
        return queue;
    }

    std::vector<Vec> fill(const cinolib::AbstractPolygonMesh<>& _mesh, const std::vector<std::optional<Vec>>& _newVerts, const Projection::Tweak& _distWeightTweak, const std::optional<std::unordered_set<Id>>& _vids, const std::optional<std::unordered_set<Id>>& _eids)
    {
        better_priority_queue::updatable_priority_queue<I, I> skippedVisQueue{ _vids
            ? createQueue(_mesh, _newVerts, *_vids)
            : createQueue(_mesh, _newVerts)
        };
        std::vector<std::optional<Vec>> newVerts{ _newVerts };
        std::vector<Real> distances;
        distances.reserve(4);

        while (!skippedVisQueue.empty())
        {
            const I vi{ skippedVisQueue.pop_value(false).key };
            const Id vid{ toId(vi) };
            const Vec vert{ _mesh.vert(vid) };
            std::vector<Id> adjVids{ _mesh.adj_v2v(vid) };
            if (_vids)
            {
                const std::unordered_set<Id>& vids{ *_vids };
                adjVids.erase(
                    std::remove_if(
                        adjVids.begin(),
                        adjVids.end(),
                        [&vids](const Id _adjVid) { return !vids.contains(_adjVid); }
                    ),
                    adjVids.end()
                );
            }
            if (_eids)
            {
                const std::unordered_set<Id>& eids{ *_eids };
                adjVids.erase(
                    std::remove_if(
                        adjVids.begin(),
                        adjVids.end(),
                        [&eids, vid, &_mesh](const Id _adjVid) { return !eids.contains(_mesh.edge_id(_adjVid, vid)); }
                    ),
                    adjVids.end()
                );
            }
            distances.clear();
            for (const Id adjVid : adjVids)
            {
                distances.push_back(vert.dist(_mesh.vert(adjVid)));
            }
            Projection::invertAndNormalizeDistances(distances);
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

        std::vector<Vec> newActualVerts{};
        newActualVerts.reserve(newVerts.size());
        std::transform(newVerts.begin(), newVerts.end(), std::back_inserter(newActualVerts), [](const std::optional<Vec>& _vec) { return *_vec; });
        return newActualVerts;
    }

}
