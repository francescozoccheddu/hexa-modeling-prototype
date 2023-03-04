#ifndef HMP_ENABLE_ALT_PROJ
#error Alternative projection not enabled
#endif

#include <HMP/Projection/altProject.hpp>

#include <cassert>
#include <lib/vertex_based_smoothing.h>
#include <utils/meshStructures.h>

namespace HMP::Projection
{

    std::vector<Vec> altProject(const Meshing::Mesher::Mesh& _source, const cinolib::AbstractPolygonMesh<>& _target, const std::vector<Utils::Point>& _pointFeats, const std::vector<Utils::EidsPath>& _pathFeats, const Options& _options)
    {
        assert(_options.alternativeMethod);
        utilities::HexahedralMesh altSource;
        utilities::TriangleMesh altTarget;
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
        return _source.vector_verts();
    }

}
