#include <HMP/Dag/Operation.hpp>
#include <HMP/Dag/Element.hpp>

namespace HMP::Dag
{

	Operation::Operation(EPrimitive _primitive)
		: Node{ EType::Operation }, m_primitive{ _primitive }
	{}

	Operation::EPrimitive Operation::primitive() const
	{
		return m_primitive;
	}

	Utils::SetView<Operation>& Operation::dependencies()
	{
		return m_dependencies;
	}

	const Utils::SetView<Operation>& Operation::dependencies() const
	{
		return m_dependencies;
	}

	Utils::SetView<Operation>& Operation::dependents()
	{
		return m_dependents;
	}

	const Utils::SetView<Operation>& Operation::dependents() const
	{
		return m_dependents;
	}

	bool& Operation::userDefined()
	{
		return m_userDefined;
	}

	bool Operation::userDefined() const
	{
		return m_userDefined;
	}

	Utils::SetView<Element>& Operation::parents()
	{
		return m_parents;
	}

	const Utils::SetView<Element>& Operation::parents() const
	{
		return m_parents;
	}

	Utils::SetView<Element>& Operation::children()
	{
		return m_children;
	}

	const Utils::SetView<Element>& Operation::children() const
	{
		return m_children;
	}

}