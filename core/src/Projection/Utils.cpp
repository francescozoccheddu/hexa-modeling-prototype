#include <HMP/Projection/Utils.hpp>

#include <cinolib/geometry/quad_utils.h>
#include <cinolib/meshes/polygonmesh.h>
#include <optional>
#include <algorithm>
#include <cinolib/export_surface.h>
#include <cpputils/collections/zip.hpp>
#include <array>

namespace HMP::Projection::Utils
{

    SurfaceExporter::SurfaceExporter(const Meshing::Mesher::Mesh& _mesh): vol{ _mesh }
    {
        cinolib::export_surface(_mesh, surf, m_v2s, m_s2v);
    }

    void SurfaceExporter::applySurfToVol()
    {
        for (Id surfVid{}; surfVid < surf.num_verts(); surfVid++)
        {
            vol.vert(m_s2v.at(surfVid)) = surf.vert(surfVid);
        }

    }
    void SurfaceExporter::applyVolToSurf()
    {
        for (Id volVid{}; volVid < vol.num_verts(); volVid++)
        {
            surf.vert(m_v2s.at(volVid)) = vol.vert(volVid);
        }
    }

    Id SurfaceExporter::toSurfVid(Id _volVid) const
    {
        return m_v2s.at(_volVid);
    }

    Id SurfaceExporter::toVolVid(Id _surfVid) const
    {
        return m_s2v.at(_surfVid);
    }

    Id SurfaceExporter::toSurfEid(Id _volEid) const
    {
        const Id volVid0{ vol.edge_vert_id(_volEid, 0) }, volVid1{ vol.edge_vert_id(_volEid, 1) };
        return surf.edge_id(toSurfVid(volVid0), toSurfVid(volVid1));
    }

    Id SurfaceExporter::toVolEid(Id _surfEid) const
    {
        const Id surfVid0{ surf.edge_vert_id(_surfEid, 0) }, surfVid1{ surf.edge_vert_id(_surfEid, 1) };
        return vol.edge_id(toVolVid(surfVid0), toVolVid(surfVid1));
    }

    std::vector<Id> SurfaceExporter::onSurfVolVids() const
    {
        std::vector<Id> vids;
        vids.reserve(m_v2s.size());
        for (const auto& [volVid, surfVid] : m_v2s)
        {
            vids.push_back(volVid);
        }
        return vids;
    }

    std::vector<Id> SurfaceExporter::onSurfVids(const Meshing::Mesher::Mesh& _mesh)
    {
        std::vector<Id> vids;
        for (Id vid{}; vid < _mesh.num_verts(); vid++)
        {
            if (_mesh.vert_is_on_srf(vid))
            {
                vids.push_back(vid);
            }
        }
        return vids;
    }

    Tweak::Tweak(Real _min, Real _power): m_min{ _min }, m_power{ _power }
    {
        if (_power < 0.0 || _power > 10.0)
        {
            throw std::logic_error{ "power out of range" };
        }
    }

    Real Tweak::min() const
    {
        return m_min;
    }

    Real Tweak::power() const
    {
        return m_power;
    }

    bool Tweak::shouldSkip(Real _value) const
    {
        return _value < m_min;
    }

    Real Tweak::apply(Real _value) const
    {
        return std::pow((_value - m_min) / (1.0 - m_min), m_power);
    }

    bool isVidsPathClosed(const std::vector<Id>& _vids)
    {
        return _vids.size() >= 2 && _vids.front() == _vids.back();
    }

    std::vector<Id> vidsPathAdjVids(const std::vector<Id>& _vids, I _i)
    {
        std::vector<Id> adjVids;
        if (_i == 0)
        {
            if (isVidsPathClosed(_vids))
            {
                adjVids.push_back(_vids[_vids.size() - 2]);
            }
        }
        else
        {
            adjVids.push_back(_vids[_i - 1]);
        }
        if (_i == _vids.size() - 1)
        {
            if (isVidsPathClosed(_vids))
            {
                adjVids.push_back(_vids[1]);
            }
        }
        else
        {
            adjVids.push_back(_vids[_i + 1]);
        }
        return adjVids;
    }

    void normalizeWeights(std::vector<Real>& _weights)
    {
        if (_weights.empty())
        {
            return;
        }
        const Real maxWeight{ *std::max_element(_weights.begin(), _weights.end()) };
        if (maxWeight != 0.0)
        {
            for (Real& w : _weights)
            {
                w /= maxWeight;
            }
        }
    }

    void invertAndNormalizeDistances(std::vector<Real>& _distances)
    {
        if (_distances.empty())
        {
            return;
        }
        const Real minDist{ *std::min_element(_distances.begin(), _distances.end()) };
        if (minDist != 0.0)
        {
            for (Real& d : _distances)
            {
                d = minDist / d;
            }
        }
        else
        {
            for (Real& d : _distances)
            {
                d = d == 0.0 ? 1.0 : 0.0;
            }
        }
    }

    std::vector<Point> toSurfFeats(const std::vector<Point>& _feats, const SurfaceExporter& _exporter)
    {
        std::vector<Point> out(_feats.size());
        for (const auto& [out, in] : cpputils::collections::zip(out, _feats))
        {
            out.sourceVid = _exporter.toSurfVid(in.sourceVid);
        }
        return out;
    }

    std::vector<EidsPath> toSurfFeats(const std::vector<EidsPath>& _feats, const SurfaceExporter& _exporter)
    {
        std::vector<EidsPath> out(_feats.size());
        for (const auto& [out, in] : cpputils::collections::zip(out, _feats))
        {
            out.sourceEids.resize(in.sourceEids.size());
            for (const auto& [outEid, inEid] : cpputils::collections::zip(out.sourceEids, in.sourceEids))
            {
                outEid = _exporter.toSurfEid(inEid);
            }
        }
        return out;
    }

    void setVerts(const std::vector<Vec>& _from, std::vector<Vec>& _to, const std::vector<Id>& _vids)
    {
        for (const auto& [vid, from] : cpputils::collections::zip(_vids, _from))
        {
            _to[toI(vid)] = from;
        }
    }

}