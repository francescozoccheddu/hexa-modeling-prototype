#pragma once

#include <HMP/types.hpp>
#include <HMP/Dag/Node.hpp>
#include <HMP/Utils/SetView.hpp>
#include <vector>

namespace HMP::Dag
{

	class Operation : public Node
	{

	public:

		using DependencySetView = HMP::Utils::SetView<Operation>;
		using DagSetView = HMP::Utils::SetView<Node, Element>;

		enum class EPrimitive
		{
			Refine, Extrude, Delete
		};

	private:

		const EPrimitive m_primitive;
		DependencySetView m_dependencies{};
		DependencySetView m_dependents{};
		bool m_userDefined{ true };

		DagSetView m_parents{ Node::parents().view<Element>() };
		DagSetView m_children{ Node::children().view<Element>() };

	protected:

		explicit Operation(EPrimitive _primitive);

	public:

		EPrimitive primitive() const;

		DependencySetView& dependencies();
		const DependencySetView& dependencies() const;
		DependencySetView& dependents();
		const DependencySetView& dependents() const;

		bool& userDefined();
		bool userDefined() const;

		DagSetView& parents();
		const DagSetView& parents() const;
		DagSetView& children();
		const DagSetView& children() const;

		std::vector<Element*> attachChildren(std::size_t _count);

	};

}