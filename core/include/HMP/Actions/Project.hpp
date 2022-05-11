#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <cinolib/meshes/trimesh.h>
#include <vector>

namespace HMP::Actions
{

	template<typename TMeshAttributes, typename TVertAttributes, typename TEdgeAttributes, typename TPolyAttributes>
	class Project final : public Commander::Action
	{

	public:

		using Trimesh = cinolib::Trimesh<TMeshAttributes, TVertAttributes, TEdgeAttributes, TPolyAttributes>;

	private:

		struct VertMove final
		{
			Vec position;
			Id vertOffset;
			Dag::Element* element;
		};

		Trimesh m_target;
		bool m_prepared;
		std::vector<VertMove> m_vertMoves;

		void apply() override;
		void unapply() override;

	public:
		
		Project(Trimesh&& _target);
		Project(const Trimesh& _target);

	};

}

#define HMP_ACTIONS_PROJECT_IMPL
#include <HMP/Actions/Project.tpp>
#undef HMP_ACTIONS_PROJECT_IMPL
