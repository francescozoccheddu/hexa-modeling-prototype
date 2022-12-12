#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <vector>

namespace HMP::Projection
{

    std::vector<Vec> percentileAdvance(const std::vector<Vec>& _from, const std::vector<Vec>& _to, const double _percentile = 0.5);

    void percentileAdvance(const std::vector<Vec>& _from, const std::vector<Vec>& _to, std::vector<Vec>& _out, const double _percentile = 0.5);

    std::vector<Vec> percentileAdvance(const Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _to);

    std::vector<Vec> percentileAdvance(const Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _to, const std::vector<Id>& _vids);

    void percentileAdvance(const Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _to, std::vector<Vec>& _out);

    void percentileAdvance(const Meshing::Mesher::Mesh& _mesh, const std::vector<Vec>& _to, const std::vector<Id>& _vids, std::vector<Vec>& _out);

}
