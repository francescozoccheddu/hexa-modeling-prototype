#include <HMP/Meshing/defensiveAdvance.hpp>

#include <algorithm>
#include <limits>

namespace HMP::Meshing
{

    std::vector<Vec> defensiveAdvance(const std::vector<Vec>& _from, const std::vector<Vec>& _to, const double _percentile)
    {
        std::vector<Vec> out;
        defensiveAdvance(_from, _to, out, _percentile);
        return out;
    }

    void defensiveAdvance(const std::vector<Vec>& _from, const std::vector<Vec>& _to, std::vector<Vec>& _out, const double _percentile)
    {
        _out.resize(_from.size());
        Real maxLength{};
        {
            std::vector<Real> lengths(_from.size());
            for (I i{}; i < _from.size(); i++)
            {
                lengths.push_back((_to[i] - _from[i]).norm());
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
        for (I i{}; i < _from.size(); i++)
        {
            const Vec offset{ _to[i] - _from[i] };
            const Vec clampedOffset{ offset.norm() <= maxLength ? offset : (offset.normalized() * maxLength) };
            _out[i] = _from[i] + clampedOffset;
        }
    }

}
