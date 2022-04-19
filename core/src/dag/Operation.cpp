#include <HMP/Dag/Operation.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Utils/span.hpp>

namespace HMP::Dag
{

	Operation::Operation(EPrimitive _primitive)
		: Node{ EType::Operation }, m_primitive{ _primitive }
	{}

	Operation::EPrimitive Operation::primitive() const
	{
		return m_primitive;
	}

	std::vector<Operation*>& Operation::dependencies()
	{
		return m_dependencies;
	}

	std::span<const Operation* const> Operation::dependencies() const
	{
		return Utils::constSpan<const Operation>(m_dependencies);
	}

	std::vector<Operation*>& Operation::dependents()
	{
		return m_dependents;
	}

	std::span<const Operation* const> Operation::dependents() const
	{
		return Utils::constSpan<const Operation>(m_dependents);
	}

	bool& Operation::userDefined()
	{
		return m_userDefined;
	}

	bool Operation::userDefined() const
	{
		return m_userDefined;
	}

	std::span<Element*> Operation::parents()
	{
		return Utils::span<Element>(Node::parents());
	}

	std::span<const Element* const> Operation::parents() const
	{
		return Utils::constSpan<const Element>(Node::parents());
	}

	std::span<Element*> Operation::children()
	{
		return Utils::span<Element>(Node::children());
	}

	std::span<const Element* const> Operation::children() const
	{
		return Utils::constSpan<const Element>(Node::children());
	}

}