#pragma once

#include <HMP/types.hpp>
#include <HMP/Dag/Node.hpp>
#include <HMP/Utils/SetView.hpp>

namespace HMP::Dag
{

	class Element : public Node
	{

	public:

		using DagSetView = HMP::Utils::SetView<Node, Operation>;

	private:

		Id m_pid{};
		PolyVerts m_vertices;

		DagSetView m_parents{ Node::parents().view<Operation>() };
		DagSetView m_children{ Node::children().view<Operation>() };

	public:

		Element();

		Id& pid();
		Id pid() const;

		PolyVerts& vertices();
		const PolyVerts& vertices() const;

		DagSetView& parents();
		const DagSetView& parents() const;
		DagSetView& children();
		const DagSetView& children() const;

	};

}