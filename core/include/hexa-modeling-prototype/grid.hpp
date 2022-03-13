#pragma once

#include <cinolib/geometry/vec_mat.h>
#include <memory>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include <string>
#include <cinolib/meshes/meshes.h>
#include <hexa-modeling-prototype/undoredo.hpp>
#include <hexa-modeling-prototype/operationstree.hpp>
#include <hexa-modeling-prototype/utils.hpp>
#include <cinolib/subdivision_legacy_hexa_schemes.h>
#include <hexa-modeling-prototype/refinementschemes.hpp>
#include <cinolib/grid_projector.h>
#include <cinolib/feature_network.h>
#include <cinolib/feature_mapping.h>
#include <cinolib/export_surface.h>
#include <cinolib/smoother.h>
#include <hexa-modeling-prototype/actions/extrudeaction.hpp>
#include <hexa-modeling-prototype/actions/removeaction.hpp>
#include <hexa-modeling-prototype/actions/refineaction.hpp>
#include <hexa-modeling-prototype/actions/facerefineaction.hpp>
#include <hexa-modeling-prototype/actions/moveaction.hpp>
#include <hexa-modeling-prototype/actions/makeconformingaction.hpp>


struct MeshGridAttributes : cinolib::Polyhedron_std_attributes
{
	unsigned int id;
};

typedef cinolib::DrawableHexmesh<cinolib::Mesh_std_attributes, cinolib::Vert_std_attributes, cinolib::Edge_std_attributes, cinolib::Polygon_std_attributes, MeshGridAttributes> MeshGrid;

enum Transform { NONE, REFLECT_XZ, REFLECT_XY, REFLECT_YZ, ROTATE_X, ROTATE_Y, ROTATE_Z };

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

	void add_operation(const std::shared_ptr<Operation>& op);

	void undo();
	void redo();

	void apply_tree(const std::list<std::shared_ptr<Operation> >& operations, unsigned int pid, bool is_user_defined = true);
	void prune_tree(const std::shared_ptr<Operation>& operation, bool is_user_defined = true);
	void fix_topology(const std::shared_ptr<Operation>& op, bool removing = false);
	void make_conforming();

	void project_on_target(cinolib::Trimesh<>& target);

	bool merge(unsigned int pid_source, unsigned int pid_dest, Transform T = NONE);


	std::map<std::vector<unsigned int>, std::shared_ptr<Element>>& id2element();
	std::unordered_map<std::shared_ptr<Element>, std::vector<unsigned int>>& element2id();

	void clear();

	void save_as_mesh(std::string filename);

private:

	std::vector<unsigned int> xz_refl_mask = { 2,1,0,3,4,5 };
	std::vector<unsigned int> xy_refl_mask = { 0,1,2,3,5,4 };
	std::vector<unsigned int> yz_refl_mask = { 0,3,2,1,4,5 };
	std::vector<unsigned int> x_rot_mask = { 4,1,5,3,2,0 };
	std::vector<unsigned int> y_rot_mask = { 0,5,2,4,1,3 };
	std::vector<unsigned int> z_rot_mask = { 3,0,1,2,4,5 };

	void extrude(unsigned int pid, unsigned int offset, const std::shared_ptr<Extrude>& extrude, bool merge_vertices = true);
	void refine(unsigned int pid, const std::shared_ptr<Refine>& refine, bool remove_father = false);
	void move(unsigned int vid, const cinolib::vec3d& displacement);
	void remove(unsigned int pid);

	void remove_refine(const std::shared_ptr<Refine>& op, std::vector<unsigned int>& polys_to_remove);
	void remove_extrude(const std::shared_ptr<Extrude>& op, std::vector<unsigned int>& polys_to_remove);
	void remove_move(std::vector<std::shared_ptr<Element>>& elements, std::vector<unsigned int>& offsets, cinolib::vec3d& displacement);
	void remove_remove(const std::shared_ptr<Remove>& op);

	void remove_operation(const std::shared_ptr<Operation>& op, std::vector<unsigned int>& polys_to_remove);
	void apply_tree_recursive(const std::list<std::shared_ptr<Operation> >& operations, unsigned int pid, bool is_user_defined = true);

	void apply_transform(std::shared_ptr<Operation>& op, Transform T);

	void update_displacement_for_op(const std::shared_ptr<Operation>& op);

	std::map<cinolib::vec3d, unsigned int, vert_compare> v_map;
	CommandManager& command_manager;

	std::deque<std::shared_ptr<Refine>> refine_queue;


	//std::unordered_map<SchemeType, std::vector<std::vector<std::vector<unsigned int>>>> st2subdivision = {{STDREF, cinolib::hex_to_grid_3x3x3}, {FIXSCM1, cinolib::hex_to_grid_2x2x2}};
	using RefinementPair = std::pair<NestedVector3Double, NestedVector3Uint>;
	std::unordered_map<SchemeType, std::pair< std::vector<std::vector<std::vector<double>>>, std::vector<std::vector<std::vector<unsigned int>>> >> st2subdivision = { {STDREF, RefinementPair(REFINEMENT::weights, REFINEMENT::vids)},
																																							  {FACESCM, RefinementPair(FACE::weights, FACE::vids)},
																																							  {EDGESCM, RefinementPair(EDGE::weights, EDGE::vids)},
																																							  {FACEREF, RefinementPair(FACEREFINEMENT::weights, FACEREFINEMENT::vids)} };



	int id2pid(const std::vector<unsigned int>& id) const;

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