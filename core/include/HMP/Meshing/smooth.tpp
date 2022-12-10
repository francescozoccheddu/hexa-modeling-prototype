#ifndef HMP_MESHING_SMOOTH_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Meshing/smooth.hpp>

namespace HMP::Meshing
{

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh)
    {
        std::vector<Vec> newVerts(toI(_mesh.num_verts()));
        for (Id vid{}; vid < _mesh.num_verts(); vid++)
        {
            Vec vertSum{};
            Real weightSum{};
            for (const Id adjVid : _mesh.adj_v2v(vid))
            {
                const Real weight = 1.0;
                vertSum += _mesh.vert(adjVid) * weight;
                weightSum += weight;
            }
            newVerts[toI(vid)] = vertSum / weightSum;
        }
        return newVerts;
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::unordered_set<Id>& _vids)
    {
        std::vector<Vec> newVerts;
        newVerts.reserve(_vids.size());
        for (const Id vid : _vids)
        {
            Vec vertSum{};
            Real weightSum{};
            for (const Id adjVid : _mesh.adj_v2v(vid))
            {
                if (_vids.contains(adjVid))
                {
                    const Real weight = 1.0;
                    vertSum += _mesh.vert(adjVid) * weight;
                    weightSum += weight;
                }
            }
            if (weightSum == 0.0)
            {
                weightSum = 1.0;
            }
            newVerts.push_back(vertSum / weightSum);
        }
        return newVerts;
    }

}
