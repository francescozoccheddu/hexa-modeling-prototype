#pragma once

#include <HMP/types.hpp>
#include <HMP/Dag/Node.hpp>
#include <vector>

namespace HMP::Dag
{

	class Operation : public Node
	{

	public:

		using Set = NodeSet<Element>;

		enum class EPrimitive
		{
			Refine, Extrude, Delete
		};

	private:

		const EPrimitive m_primitive;
		bool m_userDefined{ true };

		Set m_parents, m_children;

	protected:

		explicit Operation(EPrimitive _primitive);

	public:

		EPrimitive primitive() const;

		bool& userDefined();
		bool userDefined() const;
		
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