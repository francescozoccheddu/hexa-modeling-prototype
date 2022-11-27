#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/meshes/polygonmesh.h>
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

		TargetMesh m_target;
		bool m_prepared;
		std::vector<VertMove> m_vertMoves;

		void apply() override;
		void unapply() override;

	public:

		Project(TargetMesh&& _target);
		Project(const TargetMesh& _target);

		const TargetMesh& target() const;

	};

}
