#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Node.hpp>

namespace HMP::Dag
{

	class Element final : public Node
	{

	public:

		using Set = NodeSet<Operation>;

	private:

		PolyVerts m_vertices;

		Set m_parents, m_children;

		using Node::isElement;
		using Node::isOperation;
		using Node::element;
		using Node::operation;

	public:

		Element();

		PolyVerts& vertices();
		const PolyVerts& vertices() const;

		Set& forward(bool _descending);
		const Set& forward(bool _descending) const;
		Set& back(bool _descending);
		const Set& back(bool _descending) const;

		Set& parents();
		const Set& parents() const;
		Set& children();
		const Set& children() const;

	};

}