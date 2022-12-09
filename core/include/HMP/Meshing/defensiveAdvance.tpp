#ifndef HMP_MESHING_DEFENSIVEADVANCE_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Meshing/defensiveAdvance.hpp>

#include <algorithm>
#include <limits>
#include <numeric>

namespace HMP::Meshing
{

    template<typename M, typename V, typename E, typename P>
    std::vector<Vec> defensiveAdvance(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Vec>& _newVerts, const double _percentile)
    {
        const auto vids{ std::iota<Id>(0, _mesh.num_verts()) };
        return defensiveAdvance(_mesh, _newVerts, _percentile, vids.begin(), vids.end());
    }

    template<typename M, typename V, typename E, typename P, std::output_iterator<Id> TVidIt>
    std::vector<Vec> defensiveAdvance(const cinolib::AbstractMesh<M, V, E, P>& _mesh, const std::vector<Vec>& _newVerts, const TVidIt _vidsBegin, const TVidIt _vidsEnd, const double _percentile)
    {
        const I vidsCount{ static_cast<I>(std::distance(_vidsEnd, _vidsBegin)) };
        std::vector<Vec> newVerts;
        newVerts.reserve(vidsCount);
        Real maxLength{};
        {
            std::vector<Real> lengths;
            lengths.reserve(vidsCount);
            for (TVidIt it{ _vidsBegin }; it != _vidsEnd; ++it)
            {
                const Id vid{ *it };
                lengths.push_back((_newVerts[toI(vid)] - _mesh.vert(vid)).norm());
            }
            std::sort(lengths.begin(), lengths.end());
            I medianI{ static_cast<I>(std::round(static_cast<double>(lengths.size() - 1) * _percentile)) };
            if (medianI <= lengths.size())
            {
                maxLength = lengths[medianI];
            }
            else
            {
                maxLength = std::numeric_limits<Real>::infinity();
            }
        }
        for (TVidIt it{ _vidsBegin }; it != _vidsEnd; ++it)
        {
            const Id vid{ *it };
            const Vec vert{ _mesh.vert(vid) };
            const Vec offset{ _newVerts[toI(vid)] - vert };
            const Vec clampedOffset{ offset.norm() <= maxLength ? offset : (offset.normalized() * maxLength) };
            newVerts.push_back(vert + clampedOffset);
        }
        return newVerts;
    }

}
