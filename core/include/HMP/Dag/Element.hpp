#pragma once

#include <HMP/types.hpp>
#include <HMP/Dag/Node.hpp>
#include <HMP/Utils/SetView.hpp>

namespace HMP::Dag
{

	class Element : public Node
	{

	public:

		using Set = NodeSet<Operation>;

	private:

		Id m_pid{};
		PolyVerts m_vertices;

		Set m_parents, m_children;

	public:

		Element();

		Id& pid();
		Id pid() const;

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