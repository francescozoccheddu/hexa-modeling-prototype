#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/polygonmesh.h>
#include <vector>

namespace HMP::Algorithms::Projection
{

    class Tweak final
    {

    private:

        Real m_min;
        Real m_power;

    public:

        Tweak(Real _min, Real _power = 1.0);

        bool shouldSkip(Real _value) const;
        Real apply(Real _value) const;

    };

    enum class EInvertMode
    {
        Distance, BarycentricCoords
    };

    enum class EDisplaceMode
    {
        NormDirAvgAndDirNormAvg, NormDirAvgAndDirAvg, DirAvg, VertAvg
    };

    struct Options final
    {

        EInvertMode invertMode{ EInvertMode::Distance };
        EDisplaceMode displaceMode{ EDisplaceMode::DirAvg };
        Tweak weightTweak{ 0.0, 1.0 };
        Tweak normalDotTweak{ -1.0, 0.0 };
        Tweak unsetVertsDistWeightTweak{ 0.0, 0.0 };
        double advancePercentile{ 0.5 };
        bool smooth{ true };
        std::size_t iterations{ 5 };

    };

    std::vector<Vec> project(const cinolib::Polygonmesh<>& _source, const cinolib::Polygonmesh<>& _target, const Options& _options = {});

}
