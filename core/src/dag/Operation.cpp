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

	Operation::DependencySetView& Operation::dependencies()
	{
		return m_dependencies;
	}

	const Operation::DependencySetView& Operation::dependencies() const
	{
		return m_dependencies;
	}

	Operation::DependencySetView& Operation::dependents()
	{
		return m_dependents;
	}

	const Operation::DependencySetView& Operation::dependents() const
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

	Operation::DagSetView& Operation::parents()
	{
		return m_parents;
	}

	const Operation::DagSetView& Operation::parents() const
	{
		return m_parents;
	}

	Operation::DagSetView& Operation::children()
	{
		return m_children;
	}

	const Operation::DagSetView& Operation::children() const
	{
		return m_children;
	}

}