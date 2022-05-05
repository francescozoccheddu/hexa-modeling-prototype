#pragma once

#include <HMP/Meshing/types.hpp>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include <string>
#include <cinolib/meshes/meshes.h>
#include <HMP/Commander.hpp>
#include <HMP/Utils/Geometry.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <cinolib/subdivision_legacy_hexa_schemes.h>
#include <cinolib/grid_projector.h>
#include <cinolib/feature_network.h>
#include <cinolib/feature_mapping.h>
#include <cinolib/export_surface.h>
#include <cinolib/smoother.h>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/Refine.hpp>
#include <HMP/Actions/MoveVert.hpp>
#include <HMP/Actions/MakeConforming.hpp>

namespace HMP
{

	class Grid
	{

	public:

		struct PolyhedronAttributes final : cinolib::Polyhedron_std_attributes
		{
			Dag::Element* element;
		};

		using Mesh = cinolib::DrawableHexmesh<cinolib::Mesh_std_attributes, cinolib::Vert_std_attributes, cinolib::Edge_std_attributes, cinolib::Polygon_std_attributes, PolyhedronAttributes>;

		Grid();

		void project_on_target(cinolib::Trimesh<>& target);

		bool merge(Id pid_source, Id pid_dest);

		void clear();

		void save_as_mesh(std::string filename);

		Dag::Element& element(Id _pid);
		void element(Id _pid, Dag::Element& _element);
		const Dag::Element& element(Id _pid) const;

		PolyVerts polyVerts(Id _pid) const;
		Id addPoly(Dag::Element& _element);
		Id addPoly(const PolyVertIds& _vids, Dag::Element& _element);
		void removePoly(Id _pid);
		Id addPoly(const PolyVerts _verts, Dag::Element& _element);
		Id addOrGetVert(const Vec& _vert);
		bool getVert(const Vec& _vert, Id& _vid) const;
		Id getVert(const Vec& _vert) const;
		bool hasVert(const Vec& _vert) const;
		void vert(Id _vid, const Vec& _position);
		Id closestPolyFid(Id _pid, const Vec& _centroid) const;

		void replaceDag(Dag::Element& _root);

		Mesh& mesh();
		const Mesh& mesh() const;

	private:

		Mesh m_mesh;

		void init();

	};

}