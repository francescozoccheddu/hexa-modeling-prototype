#ifndef HMP_ENABLE_ALT_PROJ
#error alternative projection method not enabled
#endif

#include <HMP/Projection/altProject.hpp>

// fprotais' hexsmoothing
#include <lib/vertex_based_smoothing.h>
#include <utils/meshStructures.h>
// STL
#include <cassert>
#include <utility>
// cpputils
#include <cpputils/range/of.hpp>
#include <cpputils/range/index.hpp>

namespace HMP::Projection
{

    // ---------- CONVERSION UTILS ----------

    // HMP's vector to fprotais' vector
    utilities::vec3 toAltVec(const Vec& _vec)
    {
        return { _vec.x(), _vec.y(), _vec.z() };
    }

    // fprotais' vector to HMP's vector
    Vec toMainVec(const utilities::vec3& _vec)
    {
        return { _vec.x, _vec.y, _vec.z };
    }

    // HMP's hexmesh to fprotais' hexmesh
    utilities::HexahedralMesh toAltMesh(const Meshing::Mesher::Mesh& _source)
    {
        return {
            ._pts{cpputils::range::of(_source.vector_verts()).map(&toAltVec).toVector()},
            ._hexes{cpputils::range::count(_source.num_polys())
                .filter([&](const Id _pid) {
                    return !_source.poly_data(_pid).flags[cinolib::HIDDEN];
                })
                .map([&](const Id _pid) {
                    const std::vector<Id>& _poly{_source.adj_p2v(_pid)};
                    return cpputils::range::of(std::array<Id,8>{
                        _poly[0 + 0],
                        _poly[1 + 0],
                        _poly[3 + 0],
                        _poly[2 + 0],
                        _poly[0 + 4],
                        _poly[1 + 4],
                        _poly[3 + 4],
                        _poly[2 + 4],
                    })
                    .cast<int>()
                    .toArray();
                })
                .toVector()}
        };
    }

    // HMP's polygon mesh to fprotais' trimesh
    utilities::TriangleMesh toAltMesh(const cinolib::AbstractPolygonMesh<>& _target)
    {
        std::vector<std::array<int, 3>> tris;
        tris.reserve(toI(_target.num_polys()) * 3);
        for (Id pid{}; pid < _target.num_polys(); pid++)
        {
            const std::vector<Id>& vids{_target.poly_tessellation(pid)};
            for (Id t{}; t < vids.size() / 3; t++)
            {
                tris.push_back({
                    static_cast<int>(vids[3 * t + 0]),
                    static_cast<int>(vids[3 * t + 1]),
                    static_cast<int>(vids[3 * t + 2])
                    });
            }
        }
        return {
            ._pts{cpputils::range::of(_target.vector_verts()).map(&toAltVec).toVector()},
            ._tris{std::move(tris)}
        };
    }

    // --------------------------------------

    // main projection function
    // _source: the source hexmesh to project
    // _target: the target polygon mesh to project onto
    // _pointFeats: list of mapped (source point, target point) pairs
    // _pathFeats: list of mapped (source edge chain, target edge chain) pairs
    // _options: only _options.iterations matters here, all other fields are to be ignored
    // returns the new vertex positions vector, with the same size and order as _source.vector_verts()
    std::vector<Vec> altProject(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options)
    {
        assert(_options.alternativeMethod);
        utilities::HexahedralMesh altSource{toAltMesh(_source)};
        const utilities::TriangleMesh altTarget{toAltMesh(_target)};

        vertex_smoother smoother(altSource); // <-- CRASH!
        smoother.set_bnd_triangles(altTarget);

        // this is for the sharp features; can be ignored, for now
        /*
        utilities::CurveMesh altTargetCurves;
        smoother.set_locked_vertices(...);
        smoother.set_features_segment(altTargetCurves);
        for each vertex v in altSource {
            if v is in volume : continue;
            if v is CAD corner : smoother.set_vertex_point(v, coordinate);
            if v is on CAD curve : smoother.set_vertex_segments(v, listOfEdgesTheVertexCouldBeOn); // on altTargetCurves
            if v is on CAD surface : smoother.set_vertex_triangles(v, listOfTriangleTheVertexCouldBeOn); // on altTarget
        }
        */

        smoother.execute(static_cast<unsigned int>(_options.iterations));

        return cpputils::range::of(altSource._pts).map(&toMainVec).toVector();
    }

}
