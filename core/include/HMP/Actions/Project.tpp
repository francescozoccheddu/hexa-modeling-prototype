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
		const Meshing::Mesher::Mesh& src{ mesher.mesh() };
		if (!m_prepared)
		{
			m_prepared = true;
			std::cout << "Projecting..." << std::endl;

			cinolib::Quadmesh<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes> srf;

			std::unordered_map<Id, Id> src2srf, srf2src;
			cinolib::export_surface(src, srf, src2srf, srf2src);
			m_target.edge_set_flag(cinolib::MARKED, false);
			srf.edge_set_flag(cinolib::MARKED, false);
			m_target.edge_mark_sharp_creases();

			cinolib::SmootherOptions options;
			options.n_iters = 3;
			options.laplacian_mode = cinolib::UNIFORM;
			cinolib::mesh_smoother(srf, m_target, options);

			for (Id srfVid{ 0 }; srfVid < srf.num_verts(); srfVid++)
			{
				const Id vid{ srf2src[srfVid] };
				const Id pid{ src.adj_v2p(vid).front() };
				m_vertMoves.push_back({
					.position = srf.vert(srfVid),
					.vertOffset{src.poly_vert_offset(pid, vid)},
					.element{&mesher.pidToElement(pid)}
					});
			}

			m_target.clear();
		}
		std::unordered_map<Id, Vec> vertMovesMap{};
		vertMovesMap.reserve(m_vertMoves.size());
		for (VertMove& move : m_vertMoves)
		{
			const Id pid{ mesher.elementToPid(*move.element) };
			const Id vid{ src.poly_vert_id(pid, move.vertOffset) };
			const Vec oldPosition{ src.vert(vid) };
			vertMovesMap.insert({ vid, move.position });
			move.position = oldPosition;
		}
		mesher.moveVerts(vertMovesMap);
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