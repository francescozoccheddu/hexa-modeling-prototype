#include <HMP/Dag/Node.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <stdexcept>
#include <deque>

namespace HMP::Dag
{

	Node::Node(EType _type)
		: m_type{ _type }
	{}

	Node::~Node()
	{
		detachAll(true);
	}

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

	Node::SetView& Node::parents()
	{
		return m_parents;
	}

	const Node::SetView& Node::parents() const
	{
		return m_parents;
	}

	Node::SetView& Node::children()
	{
		return m_children;
	}

	const Node::SetView& Node::children() const
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

	void Node::attachParent(Node& _parent)
	{
		_parent.attachChild(*this);
	}

	void Node::detachParent(Node& _parent)
	{
		_parent.detachChild(*this);
	}

	void Node::detachParents()
	{
		for (Node& parent : m_parents)
		{
			parent.m_children.remove(*this);
		}
		m_parents.clear();
	}

	void Node::attachChild(Node& _child)
	{
		_child.m_parents.add(*this);
		m_children.add(_child);
	}

	void Node::detachChild(Node& _child, bool _deleteOrphaned)
	{
		_child.m_parents.remove(*this);
		m_children.remove(_child);
	}

	void deleteOrphaned(std::deque<Node*> _orphaned)
	{
		while (!_orphaned.empty())
		{
			Node& orphan{ *_orphaned.front() };
			_orphaned.pop_front();
			orphan.detachParents();
			for (Node& child : orphan.children())
			{
				if (child.parents().remove(orphan) && child.isRoot())
				{
					_orphaned.push_back(&child);
				}
			}
			orphan.children().clear();
			delete& orphan;
		}
	}

	void Node::detachChildren(bool _deleteOrphaned)
	{
		std::deque<Node*> orphaned{};
		for (Node& child : m_children)
		{
			if (child.m_parents.remove(*this) && child.isRoot() && _deleteOrphaned)
			{
				orphaned.push_back(&child);
			}
		}
		m_children.clear();
		deleteOrphaned(orphaned);
	}

	void Node::detachAll(bool _deleteOrphaned)
	{
		detachParents();
		detachChildren(_deleteOrphaned);
	}

}