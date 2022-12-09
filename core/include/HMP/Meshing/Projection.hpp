#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/abstract_polygonmesh.h>
#include <cinolib/meshes/abstract_polyhedralmesh.h>
#include <vector>
#include <HMP/Meshing/Mesher.hpp>

namespace HMP::Meshing::Projection
{

    class Tweak final
    {

    private:

        Real m_min;
        Real m_power;

    public:

        Tweak(Real _min, Real _power = 1.0);

        Real min() const;
        Real power() const;

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
        Tweak baseWeightTweak{ 0.0, 1.0 };
        Tweak normalDotTweak{ -1.0, 0.0 };
        Tweak unsetVertsDistWeightTweak{ 0.0, 0.0 };
        double distanceWeight{ 0.0 };
        double distanceWeightPower{ 1.0 };
        double advancePercentile{ 0.5 };
        bool smooth{ true };
        I iterations{ 5 };

    };

    struct Point final
    {
        Id sourceVid, targetVid;
    };

    struct Path final
    {
        std::vector<Id> sourceEids, targetEids;
    };

    void normalizeWeights(std::vector<Real>& _weights);
    void invertAndNormalizeDistances(std::vector<Real>& _distances);

    std::vector<Vec> project(const Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Point>& _pointFeats, const std::vector<Path>& _pathFeats, const Options& _options = {});

}
