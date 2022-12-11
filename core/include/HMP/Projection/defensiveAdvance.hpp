#pragma once

#include <HMP/Meshing/types.hpp>
#include <vector>

namespace HMP::Projection
{

    std::vector<Vec> defensiveAdvance(const std::vector<Vec>& _from, const std::vector<Vec>& _to, const double _percentile = 0.5);

    void defensiveAdvance(const std::vector<Vec>& _from, const std::vector<Vec>& _to, std::vector<Vec>& _out, const double _percentile = 0.5);

}
