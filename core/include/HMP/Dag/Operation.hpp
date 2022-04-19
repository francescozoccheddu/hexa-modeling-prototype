#pragma once

#include <HMP/Dag/Node.hpp>
#include <HMP/Utils/SetView.hpp>

namespace HMP::Dag
{

	class Operation : public Node
	{

	public:

		enum class EPrimitive
		{
			Refine, Extrude, Delete
		};

	private:

		const EPrimitive m_primitive;
		Utils::SetView<Operation> m_dependencies{};
		Utils::SetView<Operation> m_dependents{};
		bool m_userDefined{ true };

		Utils::SetView<Element> m_parents{ parents().view<Element>() };
		Utils::SetView<Element> m_children{ children().view<Element>() };

	protected:

		Operation(EPrimitive _primitive);

	public:

		EPrimitive primitive() const;

		Utils::SetView<Operation>& dependencies();
		const Utils::SetView<Operation>& dependencies() const;
		Utils::SetView<Operation>& dependents();
		const Utils::SetView<Operation>& dependents() const;

		bool& userDefined();
		bool userDefined() const;

		Utils::SetView<Element>& parents();
		const Utils::SetView<Element>& parents() const;
		Utils::SetView<Element>& children();
		const Utils::SetView<Element>& children() const;

	};

}