#include <HMP/Dag/Operation.hpp>
#include <HMP/Dag/Element.hpp>

namespace HMP::Dag
{

	Operation::Operation(EPrimitive _primitive)
		: Node{ EType::Operation }, m_primitive{ _primitive }, m_parents{ parentsHandle() }, m_children{ childrenHandle() }
	{}

	Operation::EPrimitive Operation::primitive() const
	{
		return m_primitive;
	}

	bool& Operation::userDefined()
	{
		return m_userDefined;
	}

	bool Operation::userDefined() const
	{
		return m_userDefined;
	}

	Operation::Set& Operation::forward(bool _descending)
	{
		return _descending ? m_children : m_parents;
	}

	const Operation::Set& Operation::forward(bool _descending) const
	{
		return const_cast<Operation*>(this)->forward(_descending);
	}

	Operation::Set& Operation::back(bool _descending)
	{
		return forward(!_descending);
	}

	const Operation::Set& Operation::back(bool _descending) const
	{
		return const_cast<Operation*>(this)->back(_descending);
	}

	Operation::Set& Operation::parents()
	{
		return m_parents;
	}

	const Operation::Set& Operation::parents() const
	{
		return m_parents;
	}

	Operation::Set& Operation::children()
	{
		return m_children;
	}

	const Operation::Set& Operation::children() const
	{
		return m_children;
	}

}
