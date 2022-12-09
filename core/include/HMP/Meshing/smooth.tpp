#ifndef HMP_MESHING_SMOOTH_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Meshing/smooth.hpp>

namespace HMP::Meshing
{

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh)
    {
        return smooth(_mesh, _mesh.vector_verts());
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::unordered_set<Id>& _vids)
    {
        return smooth(_mesh, _mesh.vector_verts(), _vids);
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Vec>& _verts)
    {
        std::vector<Vec> newVerts(_verts.size());
        for (I vi{}; vi < _verts.size(); vi++)
        {
            Vec vertSum{};
            Real weightSum{};
            for (const Id adjVid : _mesh.adj_v2v(toId(vi)))
            {
                const Real weight = 1.0;
                vertSum += _verts[toI(adjVid)] * weight;
                weightSum += weight;
            }
            newVerts[vi] = vertSum / weightSum;
        }
        return newVerts;
    }

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> smooth(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Vec>& _verts, const std::unordered_set<Id>& _vids)
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
                    vertSum += _verts[toI(adjVid)] * weight;
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
