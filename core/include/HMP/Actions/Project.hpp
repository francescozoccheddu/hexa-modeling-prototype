#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/meshes/polygonmesh.h>
#include <HMP/Meshing/Projection.hpp>
#include <vector>

namespace HMP::Actions
{

	class Project final : public Commander::Action
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
		const Meshing::Projection::Options m_options;

		void apply() override;
		void unapply() override;

	public:

		Project(TargetMesh&& _target, const Meshing::Projection::Options& _options);
		Project(const TargetMesh& _target, const Meshing::Projection::Options& _options);

		const TargetMesh& target() const;

		const Meshing::Projection::Options options() const;

	};

}
