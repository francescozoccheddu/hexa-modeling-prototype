#ifndef HMP_ENABLE_ALT_PROJ
#error Alternative projection not enabled
#endif

#include <HMP/Projection/altProject.hpp>

#include <cassert>
#include <lib/vertex_based_smoothing.h>
#include <utils/meshStructures.h>
#include <cpputils/range/of.hpp>
#include <cpputils/range/index.hpp>
#include <utility>

namespace HMP::Projection
{

    utilities::vec3 toAltVec(const Vec& _vec)
    {
        return { _vec.x(), _vec.y(), _vec.z() };
    }

    Vec toMainVec(const utilities::vec3& _vec)
    {
        return { _vec.x, _vec.y, _vec.z };
    }

    utilities::HexahedralMesh toAltMesh(const Meshing::Mesher::Mesh& _source)
    {
        return {
            ._pts{cpputils::range::of(_source.vector_verts()).map(&toAltVec).toVector()},
            ._hexes{cpputils::range::of(_source.vector_polys()).map([](const std::vector<Id>& _poly) {
                return cpputils::range::of(std::array<Id,8>{
                    _poly[0 + 0],
                    _poly[1 + 0],
                    _poly[3 + 0],
                    _poly[2 + 0],
                    _poly[0 + 4],
                    _poly[1 + 4],
                    _poly[3 + 4],
                    _poly[2 + 4]
                }).cast<int>().toArray();
            }).toVector()}
        };
    }

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

    std::vector<Vec> altProject(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options)
    {
        assert(_options.alternativeMethod);
        utilities::HexahedralMesh altSource{toAltMesh(_source)};
        utilities::TriangleMesh altTarget{toAltMesh(_target)};
        //utilities::CurveMesh curvesToProjectTo;

        vertex_smoother smoother(altSource);
        //smoother.set_locked_vertices(...);
        smoother.set_bnd_triangles(altTarget);
        //smoother.set_features_segment(curvesToProjectTo);

        /*for each vertex v in yourMesh {
            if v is in volume : continue;
            if v is CAD corner : smoother.set_vertex_point(v, coordinate);
            if v is on CAD curve : smoother.set_vertex_segments(v, listOfEdgesTheVertexCouldBeOn); // on curvesToProjectTo
            if v is on CAD surface : smoother.set_vertex_triangles(v, listOfTriangleTheVertexCouldBeOn); // on surfaceToProjectTo
        }*/

        smoother.execute(static_cast<unsigned int>(_options.iterations));
        return cpputils::range::of(altSource._pts).map(&toMainVec).toVector();
    }

}
