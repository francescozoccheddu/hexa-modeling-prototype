#include <HMP/Actions/Project.hpp>

#include <utility>
#include <unordered_map>
#include <cinolib/export_surface.h>
#include <HMP/Algorithms/Projection.hpp>

namespace HMP::Actions
{

	void Project::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& source{ mesher.mesh() };
		if (!m_prepared)
		{
			m_prepared = true;
			cinolib::Polygonmesh<> sourceSurf;
			std::unordered_map<Id, Id> sourceSurf2Source, source2SourceSurf;
			cinolib::export_surface(source, sourceSurf, source2SourceSurf, sourceSurf2Source);
			const std::vector<Vec> newSourceSurfVerts{ Algorithms::Projection::project(sourceSurf, m_target, m_options) };
			m_vertMoves.resize(newSourceSurfVerts.size());
			for (I sourceSurfVi{}; sourceSurfVi < newSourceSurfVerts.size(); sourceSurfVi++)
			{
				const Id sourceVid{ sourceSurf2Source[i2id(sourceSurfVi)] };
				const Id sourcePid{ source.adj_v2p(sourceVid).front() };
				const Id sourceVertOffset{ source.poly_vert_offset(sourcePid, sourceVid) };
				m_vertMoves[sourceSurfVi] = VertMove{
					.position = newSourceSurfVerts[sourceSurfVi],
					.vertOffset = sourceVertOffset,
					.element = &mesher.pidToElement(sourcePid)
				};
			}
		}
		std::unordered_map<Id, Vec> vertMovesMap{};
		vertMovesMap.reserve(m_vertMoves.size());
		for (VertMove& move : m_vertMoves)
		{
			const Id pid{ mesher.elementToPid(*move.element) };
			const Id vid{ source.poly_vert_id(pid, move.vertOffset) };
			const Vec oldPosition{ source.vert(vid) };
			vertMovesMap.insert({ vid, move.position });
			move.position = oldPosition;
		}
		mesher.moveVerts(vertMovesMap);
		mesher.updateMesh();
	}

	void Project::unapply()
	{
		apply();
	}

	Project::Project(TargetMesh&& _target, const Algorithms::Projection::Options& _options)
		: m_target{ std::move(_target) }, m_vertMoves{}, m_prepared{ false }, m_options{ _options }
	{}

	Project::Project(const TargetMesh& _target, const Algorithms::Projection::Options& _options)
		: m_target{ _target }, m_vertMoves{}, m_prepared{ false }, m_options{ _options }
	{}

	const Project::TargetMesh& Project::target() const
	{
		return m_target;
	}

	const Algorithms::Projection::Options Project::options() const
	{
		return m_options;
	}

}