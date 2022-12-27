#include <HMP/Actions/Project.hpp>

#include <utility>

namespace HMP::Actions
{

	void Project::apply()
	{
		/*Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& source{ mesher.mesh() };
		if (!m_prepared)
		{
			m_prepared = true;
			const std::vector<Vec> newVerts{ Projection::project(source, m_target, m_pointFeats, m_pathFeats, m_options) };
			m_vertMoves.resize(newVerts.size());
			for (I sourceVi{}; sourceVi < newVerts.size(); sourceVi++)
			{
				const Id sourceVid{ toId(sourceVi) };
				const Id sourcePid{ source.adj_v2p(sourceVid).front() };
				const Id sourceVertOffset{ source.poly_vert_offset(sourcePid, sourceVid) };
				m_vertMoves[sourceVi] = VertMove{
					.position = newVerts[sourceVi],
					.vertOffset = sourceVertOffset,
					.element = &mesher.pidToElement(sourcePid)
				};
			}
		}
		for (VertMove& move : m_vertMoves)
		{
			const Id pid{ mesher.elementToPid(*move.element) };
			const Id vid{ source.poly_vert_id(pid, move.vertOffset) };
			const Vec oldPosition{ source.vert(vid) };
			mesher.moveVert(vid, move.position);
			move.position = oldPosition;
		}
		mesher.updateMesh();*/
	}

	void Project::unapply()
	{
		apply();
	}

	Project::Project(TargetMesh&& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options)
		: m_target{ std::move(_target) }, m_pointFeats{ _pointFeats }, m_pathFeats{ _pathFeats }, m_options{ _options }, m_prepared{ false }
	{}

	Project::Project(const TargetMesh& _target, const std::vector<Projection::Utils::Point>& _pointFeats, const std::vector<Projection::Utils::EidsPath>& _pathFeats, const Projection::Options& _options)
		: m_target{ _target }, m_pointFeats{ _pointFeats }, m_pathFeats{ _pathFeats }, m_options{ _options }, m_prepared{ false }
	{}

	const Project::TargetMesh& Project::target() const
	{
		return m_target;
	}

	const Projection::Options Project::options() const
	{
		return m_options;
	}

}