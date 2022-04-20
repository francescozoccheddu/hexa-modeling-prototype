#pragma once

#include <cinolib/geometry/vec_mat.h>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include <string>
#include <cinolib/meshes/meshes.h>
#include <HMP/undoredo.hpp>
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
#include <HMP/actions/extrudeaction.hpp>
#include <HMP/actions/removeaction.hpp>
#include <HMP/actions/refineaction.hpp>
#include <HMP/actions/facerefineaction.hpp>
#include <HMP/actions/moveaction.hpp>
#include <HMP/actions/makeconformingaction.hpp>

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

		void move_vert(unsigned int vid, const cinolib::vec3d& displacement);
		void move_edge(unsigned int eid, const cinolib::vec3d& displacement);
		void move_face(unsigned int fid, const cinolib::vec3d& displacement);

		void add_refine(unsigned int pid);
		void add_face_refine(unsigned int fid);
		void add_extrude(unsigned int pid, unsigned int face_offset);
		void add_move(unsigned int vid, const cinolib::vec3d& displacement);
		void add_remove(unsigned int pid);

		void add_operation(const Dag::Operation& op);

		void undo();
		void redo();

		void apply_tree(const std::vector<Dag::Operation*>& operations, unsigned int pid, bool is_user_defined = true);
		void prune_tree(const Dag::Operation& operation, bool is_user_defined = true);
		void fix_topology(const Dag::Operation& op, bool removing = false);
		void make_conforming();

		void project_on_target(cinolib::Trimesh<>& target);

		bool merge(unsigned int pid_source, unsigned int pid_dest);

		void clear();

		void save_as_mesh(std::string filename);

	private:

		void extrude(unsigned int pid, unsigned int offset, Dag::Extrude& extrude, bool merge_vertices = true);
		void refine(unsigned int pid, Dag::Refine& refine, bool remove_father = false);
		void move(unsigned int vid, const cinolib::vec3d& displacement);
		void remove(unsigned int pid);

		void remove_refine(const Dag::Refine& op, std::vector<unsigned int>& polys_to_remove);
		void remove_extrude(const Dag::Extrude& op, std::vector<unsigned int>& polys_to_remove);
		void remove_move(std::vector<Dag::Element*>& elements, std::vector<unsigned int>& offsets, cinolib::vec3d& displacement);
		void remove_remove(const Dag::Delete& op);

		void remove_operation(const Dag::Operation& op, std::vector<unsigned int>& polys_to_remove);
		void apply_tree_recursive(const std::vector<Dag::Operation*>& operations, unsigned int pid, bool is_user_defined = true);

		void apply_transform(Dag::Operation& op);

		void update_displacement_for_op(Dag::Operation& op);

		std::map<cinolib::vec3d, unsigned int, Utils::Geometry::VertexComparer> v_map;
		CommandManager& command_manager;

		std::deque<Dag::Refine*> refine_queue;

		std::pair<int, unsigned int> direction_to_axis(const cinolib::vec3d& dir);


		void update_mesh();
		void init();

		friend class ExtrudeAction;
		friend class RemoveAction;
		friend class RefineAction;
		friend class FaceRefineAction;
		friend class MoveAction;
		friend class MakeConformingAction;

	};

}