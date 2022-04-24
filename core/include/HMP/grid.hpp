#pragma once

#include <HMP/types.hpp>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include <string>
#include <cinolib/meshes/meshes.h>
#include <HMP/Commander.hpp>
#include <HMP/operationstree.hpp>
#include <HMP/Utils/Geometry.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <cinolib/subdivision_legacy_hexa_schemes.h>
#include <cinolib/grid_projector.h>
#include <cinolib/feature_network.h>
#include <cinolib/feature_mapping.h>
#include <cinolib/export_surface.h>
#include <cinolib/smoother.h>
#include <HMP/actions/Delete.hpp>
#include <HMP/actions/Extrude.hpp>
#include <HMP/actions/Refine.hpp>
#include <HMP/actions/MoveVert.hpp>
#include <HMP/actions/MakeConforming.hpp>

namespace HMP
{

	struct PolyhedronAttributes final : cinolib::Polyhedron_std_attributes
	{
		Dag::Element* element;
	};

	typedef cinolib::DrawableHexmesh<cinolib::Mesh_std_attributes, cinolib::Vert_std_attributes, cinolib::Edge_std_attributes, cinolib::Polygon_std_attributes, PolyhedronAttributes> MeshGrid;

	class Grid
	{

	public:
		Grid();
		MeshGrid mesh;
		OperationsTree op_tree;

		void move_vert(Id vid, const Vec& displacement);
		void move_edge(Id eid, const Vec& displacement);
		void move_face(Id fid, const Vec& displacement);

		void add_refine(Id pid);
		void add_face_refine(Id fid);
		void add_extrude(Id pid, Id face_offset);
		void add_move(Id vid, const Vec& displacement);
		void add_remove(Id pid);

		void add_operation(const Dag::Operation& op);

		void undo();
		void redo();

		void apply_tree(const std::vector<Dag::Operation*>& operations, Id pid, bool is_user_defined = true);
		void prune_tree(const Dag::Operation& operation, bool is_user_defined = true);
		void fix_topology(const Dag::Operation& op, bool removing = false);
		void make_conforming();

		void project_on_target(cinolib::Trimesh<>& target);

		bool merge(Id pid_source, Id pid_dest);

		void clear();

		void save_as_mesh(std::string filename);

		/* ------------- new-dag ------------- */

		Commander& commander();
		const Commander& commander() const;

		Dag::Element& element(Id _pid);
		void element(Id _pid, Dag::Element& _element);
		const Dag::Element& element(Id _pid) const;

		PolyVerts polyVertsFromFace(Id _pid, Id _faceOffset) const;
		PolyVerts polyVertsFromEdge(Id _pid, Id _eid) const;
		PolyVerts polyVerts(Id _pid) const;
		Id addPoly(Dag::Element& _element);
		Id addPoly(const PolyIds& _vids, Dag::Element& _element);
		void removePoly(Id _pid);
		Id addPoly(const PolyVerts _verts, Dag::Element& _element);
		Id addOrGetVert(const Vec&  _vert);
		bool getVert(const Vec&  _vert, Id& _vid) const;
		Id getVert(const Vec&  _vert) const;
		bool hasVert(const Vec&  _vert) const;
		void vert(Id _vid, const Vec& _position);
		Id closestPolyFid(Id _pid, const Vec& _centroid) const;

		/* ^^^^^^^^^^^^^ new-dag ^^^^^^^^^^^^^ */

	private:

		void extrude(Id _pid, Id _faceOffset, Dag::Extrude& _operation);
		void refine(Id pid, Dag::Refine& refine, bool remove_father = false);
		void move(Id vid, const Vec& displacement);

		void remove_refine(const Dag::Refine& op, std::vector<Id>& polys_to_remove);
		void remove_extrude(const Dag::Extrude& op, std::vector<Id>& polys_to_remove);
		void remove_move(std::vector<Dag::Element*>& elements, std::vector<Id>& offsets, Vec& displacement);
		void remove_remove(const Dag::Delete& op);

		void remove_operation(const Dag::Operation& op, std::vector<Id>& polys_to_remove);
		void apply_tree_recursive(const std::vector<Dag::Operation*>& operations, Id pid, bool is_user_defined = true);

		void apply_transform(Dag::Operation& op);

		void update_displacement_for_op(Dag::Operation& op);

		std::map<Vec, Id, Utils::Geometry::VertexComparer> v_map;

		std::deque<Dag::Refine*> refine_queue;

		std::pair<int, Id> direction_to_axis(const Vec& dir);

		Commander m_commander{ *this };

		void update_mesh();
		void init();

		friend class Actions::Extrude;
		friend class Actions::Delete;
		friend class Actions::Refine;
		friend class Actions::MoveVert;
		friend class Actions::MakeConforming;

	};

}