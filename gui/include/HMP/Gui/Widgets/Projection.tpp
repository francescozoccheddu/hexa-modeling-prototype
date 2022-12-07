#ifndef HMP_GUI_WIDGETS_PROJECTION_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Gui/Widgets/Projection.hpp>

#include <functional>

namespace HMP::Gui::Widgets
{

    template<class M, class V, class E, class P>
    void Projection::setCreaseEdgeAtPoint(const Vec& _point, bool _add, const cinolib::AbstractMesh<M, V, E, P>& _mesh, bool _source)
    {
        if (!m_showCreases || m_showAllCreases || m_creases.empty())
        {
            return;
        }
        EdgeChainPair& creasePair{ m_creases[m_currentCrease] };
        EdgeChain& crease{ _source ? creasePair.source : creasePair.target };
        const bool has1{ crease.size() > 0 }, has2{ crease.size() > 1 }, has3{ crease.size() > 2 };
        const I lastI{ crease.size() - 1 };
        const std::vector<Id> endEids{ has1 ? has2 ? std::vector<Id>{crease[0], crease[lastI]} : std::vector<Id>{ crease[0] } : std::vector<Id>{} };
        const bool closed{ has3 && _mesh.edges_are_adjacent(endEids[0], endEids[1]) };
        if (_add)
        {
            if (closed)
            {
                return;
            }
            Id closestEid{ noId };
            Real closestDist{ std::numeric_limits<Real>::infinity() };
            if (!has1)
            {
                for (Id eid{}; eid < _mesh.num_edges(); eid++)
                {
                    if (has2 && (eid == crease[1] || eid == crease[lastI - 1]))
                    {
                        continue;
                    }
                    if (_source && !m_mesher.mesh().edge_is_on_srf(eid))
                    {
                        continue;
                    }
                    const Real dist{ _point.dist_sqrd(_mesh.edge_sample_at(eid, 0.5)) };
                    if (dist < closestDist)
                    {
                        closestDist = dist;
                        closestEid = eid;
                    }
                }
            }
            else
            {
                for (const Id endEid : endEids)
                {
                    for (const Id eid : _mesh.adj_e2e(endEid))
                    {
                        if (has2 && (eid == crease[1] || eid == crease[lastI - 1]))
                        {
                            continue;
                        }
                        const Real dist{ _point.dist_sqrd(_mesh.edge_sample_at(eid, 0.5)) };
                        if (dist < closestDist)
                        {
                            closestDist = dist;
                            closestEid = eid;
                        }
                    }
                }
            }
            if (closestEid == noId)
            {
                return;
            }
            if (has2)
            {
                std::set<Id> invalidVids;
                if (has3)
                {
                    for (I i{ 1 }; i < lastI; i++)
                    {
                        const Id eid{ crease[i] };
                        invalidVids.insert(_mesh.edge_vert_id(eid, 0));
                        invalidVids.insert(_mesh.edge_vert_id(eid, 1));
                    }
                }
                else
                {
                    invalidVids.insert(_mesh.vert_shared(endEids[0], endEids[1]));
                }
                if (invalidVids.contains(_mesh.edge_vert_id(closestEid, 0)) || invalidVids.contains(_mesh.edge_vert_id(closestEid, 1)))
                {
                    return;
                }
            }
            if (!has1 || !_mesh.edges_are_adjacent(crease[0], closestEid))
            {
                crease.push_back(closestEid);
            }
            else
            {
                crease.insert(crease.begin(), closestEid);
            }
            const cinolib::Color color{ cinolib::Color::hsv2rgb(static_cast<float>(m_currentCrease) / static_cast<float>(m_creases.size()), 1.0f, 1.0f) };
            if (_source)
            {
                m_mesher.paintEdge(closestEid, color);
            }
            else
            {
                m_targetWidget.paintEdge(closestEid, color);
            }
        }
        else
        {
            const std::vector<Id>& candidates{ closed ? crease : endEids };
            Real closestDist{ std::numeric_limits<Real>::infinity() };
            Id closestEid{ noId };
            for (const Id eid : candidates)
            {
                const Real dist{ _point.dist_sqrd(_mesh.edge_sample_at(eid, 0.5)) };
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closestEid = eid;
                }
            }
            if (closestEid == noId)
            {
                return;
            }
            const I i{ static_cast<I>(std::find(crease.begin(), crease.end(), closestEid) - crease.begin()) };
            const I nextI{ (i + 1) % crease.size() };
            std::rotate(crease.begin(), crease.begin() + nextI, crease.end());
            crease.pop_back();
            if (_source)
            {
                m_mesher.unpaintEdge(closestEid);
            }
            else
            {
                m_targetWidget.unpaintEdge(closestEid);
            }
        }
    }


}
