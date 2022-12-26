#include <HMP/Dag/Node.hpp>

#include <cassert>

namespace HMP::Dag
{

	Node::Node(EType _type):
		m_type{ _type },
		m_parentsImpl{}, m_childrenImpl{},
		m_parents{
			m_parentsImpl,
			[this](auto && ..._args) { return onParentAttach(_args...); },
			[this](auto && ..._args) { return onParentDetach(_args...); },
			[this](auto && ..._args) { return onParentsDetachAll(_args...); }
		},
		m_children{
			m_childrenImpl,
			[this](auto && ..._args) { return onChildAttach(_args...); },
			[this](auto && ..._args) { return onChildDetach(_args...); },
			[this](auto && ..._args) { return onChildrenDetachAll(_args...); }
		},
		m_handles{ 0 }
	{}

	void Node::deleteDangling(std::queue<Node*>& _dangling, bool _descending)
	{
		while (!_dangling.empty())
		{
			Node& node{ *_dangling.front() };
			_dangling.pop();
			assert(node.back(_descending).empty());
			for (Node& next : node.forward(_descending))
			{
				assert(next.back(_descending).data().remove(node));
				if (next.back(_descending).empty() && !next.m_handles)
				{
					_dangling.push(&next);
				}
			}
			node.forward(_descending).data().clear();
			delete& node;
		}
	}

	bool Node::onAttach(Node& _node, bool _descending)
	{
		if (forward(_descending).data().add(_node))
		{
			assert(_node.back(_descending).data().add(*this));
			return true;
		}
		return false;
	}

	bool Node::onDetach(Node& _node, bool _deleteDangling, bool _descending)
	{
		if (forward(_descending).data().remove(_node))
		{
			assert(_node.back(_descending).data().remove(*this));
			if (_deleteDangling && _node.back(_descending).empty())
			{
				std::queue<Node*> dangling{};
				dangling.push(&_node);
				deleteDangling(dangling, _descending);
			}
			return true;
		}
		return false;
	}

	bool Node::onDetachAll(bool _deleteDangling, bool _descending)
	{
		std::queue<Node*> dangling{};
		const bool wasEmpty{ forward(_descending).empty() };
		for (Node& next : forward(_descending))
		{
			assert(next.back(_descending).data().remove(*this));
			if (_deleteDangling && next.back(_descending).empty())
			{
				dangling.push(&next);
			}
		}
		forward(_descending).data().clear();
		deleteDangling(dangling, _descending);
		return !wasEmpty;
	}

	bool Node::onParentAttach(Node& _parent)
	{
		onParentAttaching(_parent);
		return onAttach(_parent, false);
	}

	bool Node::onParentDetach(Node& _parent, bool _deleteDangling)
	{
		return onDetach(_parent, _deleteDangling, false);
	}

	bool Node::onParentsDetachAll(bool _deleteDangling)
	{
		return onDetachAll(_deleteDangling, false);
	}

	bool Node::onChildAttach(Node& _child)
	{
		onChildAttaching(_child);
		return onAttach(_child, true);
	}

	bool Node::onChildDetach(Node& _child, bool _deleteDangling)
	{
		return onDetach(_child, _deleteDangling, true);
	}

	bool Node::onChildrenDetachAll(bool _deleteDangling)
	{
		return onDetachAll(_deleteDangling, true);
	}

	void Node::onParentAttaching(Node& _child) const {}

	void Node::onChildAttaching(Node& _child) const {}

	Internal::NodeSetHandle& Node::parentsHandle()
	{
		return m_parents.handle();
	}

	Internal::NodeSetHandle& Node::childrenHandle()
	{
		return m_children.handle();
	}

	Node::~Node()
	{
		m_parents.detachAll(false);
		m_children.detachAll(false);
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

	bool Node::isRoot() const
	{
		return m_parents.empty();
	}

	bool Node::isLeaf() const
	{
		return m_children.empty();
	}

	Element& Node::element()
	{
		return reinterpret_cast<Element&>(*this);
	}

	const Element& Node::element() const
	{
		return reinterpret_cast<const Element&>(*this);
	}

	Operation& Node::operation()
	{
		return reinterpret_cast<Operation&>(*this);
	}

	const Operation& Node::operation() const
	{
		return reinterpret_cast<const Operation&>(*this);
	}

	Node::Set& Node::forward(bool _descending)
	{
		return _descending ? m_children : m_parents;
	}

	const Node::Set& Node::forward(bool _descending) const
	{
		return const_cast<Node*>(this)->forward(_descending);
	}

	Node::Set& Node::back(bool _descending)
	{
		return forward(!_descending);
	}

	const Node::Set& Node::back(bool _descending) const
	{
		return const_cast<Node*>(this)->back(_descending);
	}

	Node::Set& Node::parents()
	{
		return m_parents;
	}

	const Node::Set& Node::parents() const
	{
		return m_parents;
	}

	Node::Set& Node::children()
	{
		return m_children;
	}

	const Node::Set& Node::children() const
	{
		return m_children;
	}

}