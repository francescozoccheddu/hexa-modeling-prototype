#ifndef HMP_MESHING_SMOOTH_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Meshing/smooth.hpp>

#include <cinolib/parallel_for.h>

namespace HMP::Meshing
{

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh)
    {
        std::vector<Vec> newVerts(toI(_mesh.num_verts()));
        const auto func{ [&_mesh, &newVerts](Id _vid) {
            Vec vertSum{};
            Real weightSum{};
            for (const Id adjVid : _mesh.adj_v2v(_vid))
            {
                const Real weight = 1.0;
                vertSum += _mesh.vert(adjVid) * weight;
                weightSum += weight;
            }
            newVerts[toI(_vid)] = vertSum / weightSum;
        } };
        cinolib::PARALLEL_FOR(0, _mesh.num_verts(), 256, func);
        return newVerts;
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::unordered_set<Id>& _vids, const std::unordered_set<Id>& _eids)
    {
        std::vector<Vec> newVerts;
        newVerts.reserve(_vids.size());
        for (const Id vid : _vids)
        {
            std::vector<Id> adjVids{ _mesh.adj_v2v(vid) };
            adjVids.erase(
                std::remove_if(
                    adjVids.begin(),
                    adjVids.end(),
                    [&_vids](const Id _adjVid) { return !_vids.contains(_adjVid); }
                ),
                adjVids.end()
            );
            adjVids.erase(
                std::remove_if(
                    adjVids.begin(),
                    adjVids.end(),
                    [&_eids, vid, &_mesh](const Id _adjVid) { return !_eids.contains(_mesh.edge_id(_adjVid, vid)); }
                ),
                adjVids.end()
            );
            if (adjVids.empty())
            {
                newVerts.push_back(_mesh.vert(vid));
            }
            else
            {
                Vec vertSum{};
                Real weightSum{};
                for (const Id adjVid : adjVids)
                {
                    const Real weight = 1.0;
                    vertSum += _mesh.vert(adjVid) * weight;
                    weightSum += weight;
                }
                newVerts.push_back(vertSum / weightSum);
            }
        }
        return newVerts;
    }

}
