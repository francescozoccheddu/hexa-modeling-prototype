#pragma once

#include <hexa-modeling-prototype/dag/Node.hpp>
#include <vector>
#include <span>

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
		std::vector<Operation*> m_dependencies{};
		std::vector<Operation*> m_dependents{};
		bool m_userDefined{ true };

	protected:

		Operation(EPrimitive _primitive);

	public:

		EPrimitive primitive() const;

		std::vector<Operation*>& dependencies();
		std::span<const Operation* const> dependencies() const;
		std::vector<Operation*>& dependents();
		std::span<const Operation* const> dependents() const;

		bool& userDefined();
		bool userDefined() const;

		std::span<Element*> parents();
		std::span<const Element* const> parents() const;
		std::span<Element*> children();
		std::span<const Element* const> children() const;

	};

}