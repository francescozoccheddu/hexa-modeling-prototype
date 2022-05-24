#ifndef HMP_ACTIONS_PROJECT_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Actions/Project.hpp>

#include <utility>
#include <unordered_map>
#include <cinolib/meshes/quadmesh.h>
#include <cinolib/export_surface.h>
#include <cinolib/octree.h>
#include <cinolib/smoother.h>

namespace HMP::Actions
{

	template<typename TMeshAttributes, typename TVertAttributes, typename TEdgeAttributes, typename TPolyAttributes>
	void Project<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes>::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		if (!m_prepared)
		{
			m_prepared = true;
			std::cout << "Projecting..." << std::endl;

			cinolib::Quadmesh<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes> peel;
			//std::unordered_map<Id, Id> h2q, q2h;
			//cinolib::export_surface(mesh, peel, h2q, q2h);

			std::unordered_map<Id, Id> m_map, v_map;
			cinolib::export_surface(mesh, peel, m_map, v_map);
			m_target.edge_set_flag(cinolib::MARKED, false);
			peel.edge_set_flag(cinolib::MARKED, false);
			cinolib::Quadmesh<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes> tmp_peel = peel;
			m_target.edge_mark_sharp_creases();

			cinolib::SmootherOptions options;
			options.n_iters = 3;
			options.laplacian_mode = cinolib::UNIFORM;
			cinolib::mesh_smoother(peel, m_target, options);

			for (Id vid{ 0 }; vid < peel.num_verts(); vid++)
			{
				if (!(mesh.vert(v_map[vid]) == peel.vert(vid)))
				{
					const Id pid{ mesh.adj_v2p(vid).front() };
					m_vertMoves.push_back({
						.position = peel.vert(vid),
						.vertOffset{mesh.poly_vert_offset(pid, vid)},
						.element{&mesher.pidToElement(pid)}
						});
				}
			}

			m_target.clear();
		}
		for (VertMove& move : m_vertMoves)
		{
			const Id pid{ mesher.elementToPid(*move.element) };
			const Id vid{ mesh.poly_vert_id(pid, move.vertOffset) };
			const Vec oldPosition{ mesh.vert(vid) };
			mesher.moveVert(vid, move.position);
			move.position = oldPosition;
		}
		mesher.updateMesh();
	}

	template<typename TMeshAttributes, typename TVertAttributes, typename TEdgeAttributes, typename TPolyAttributes>
	void Project<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes>::unapply()
	{
		apply();
	}

	template<typename TMeshAttributes, typename TVertAttributes, typename TEdgeAttributes, typename TPolyAttributes>
	Project<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes>::Project(Trimesh&& _target)
		: m_target{ std::move(_target) }, m_vertMoves{}, m_prepared{ false }
	{}

	template<typename TMeshAttributes, typename TVertAttributes, typename TEdgeAttributes, typename TPolyAttributes>
	Project<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes>::Project(const Trimesh& _target)
		: m_target{ _target }, m_vertMoves{}, m_prepared{ false }
	{}

	template<typename TMeshAttributes, typename TVertAttributes, typename TEdgeAttributes, typename TPolyAttributes>
	const typename Project<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes>::Trimesh& Project<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes>::target() const
	{
		return m_target;
	}

}