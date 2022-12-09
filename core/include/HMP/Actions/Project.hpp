#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/meshes/polygonmesh.h>
#include <HMP/Meshing/Projection.hpp>
#include <vector>

namespace HMP::Actions
{

	class Project final: public Commander::Action
	{

	public:

		using TargetMesh = cinolib::Polygonmesh<>;

	private:

		struct VertMove final
		{
			Vec position;
			Id vertOffset;
			Dag::Element* element;
		};

		const TargetMesh m_target;
		bool m_prepared;
		std::vector<VertMove> m_vertMoves;
		const std::vector<Meshing::Projection::Point> m_pointFeats;
		const std::vector<Meshing::Projection::Path> m_pathFeats;
		const Meshing::Projection::Options m_options;

		void apply() override;
		void unapply() override;

	public:

		Project(TargetMesh&& _target, const std::vector<Meshing::Projection::Point>& _pointFeats, const std::vector<Meshing::Projection::Path>& _pathFeats, const Meshing::Projection::Options& _options);
		Project(const TargetMesh& _target, const std::vector<Meshing::Projection::Point>& _pointFeats, const std::vector<Meshing::Projection::Path>& _pathFeats, const Meshing::Projection::Options& _options);

		const TargetMesh& target() const;

		const Meshing::Projection::Options options() const;

	};

}
