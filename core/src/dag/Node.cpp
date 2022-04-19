#include <HMP/Dag/Node.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <stdexcept>

namespace HMP::Dag
{

	Node::Node(EType _type)
		: m_type{ _type }
	{}

	Node::EType Node::type() const
	{
		return m_type;
	}

	bool Node::isElement() const
	{
		return m_type == EType::Element;
	}

	bool Node::isOperation() const
	{
		return m_type == EType::Operation;
	}

	Element& Node::element()
	{
		if (!isElement())
		{
			throw std::logic_error{ "not an element" };
		}
		return static_cast<Element&>(*this);
	}

	const Element& Node::element() const
	{
		return const_cast<Node*>(this)->element();
	}

	Operation& Node::operation()
	{
		if (!isOperation())
		{
			throw std::logic_error{ "not an operation" };
		}
		return static_cast<Operation&>(*this);
	}

	const Operation& Node::operation() const
	{
		return const_cast<Node*>(this)->operation();
	}

	Utils::SetView<Node>& Node::parents()
	{
		return m_parents;
	}

	const Utils::SetView<Node>& Node::parents() const
	{
		return m_parents;
	}

	Utils::SetView<Node>& Node::children()
	{
		return m_children;
	}

	const Utils::SetView<Node>& Node::children() const
	{
		return m_children;
	}

	bool Node::isRoot() const
	{
		return m_parents.empty();
	}

	bool Node::isLeaf() const
	{
		return m_children.empty();
	}

}