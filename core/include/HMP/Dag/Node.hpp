#pragma once

#include <HMP/Dag/NodeSet.hpp>
#include <type_traits>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <queue>

namespace HMP::Dag
{

	class Element;
	class Operation;

	class Node : public cpputils::mixins::ReferenceClass
	{

	public:

		using Set = NodeSet<Node>;

		enum class EType
		{
			Element, Operation
		};

	private:

		const EType m_type;

		Internal::NodeSetData m_parentsImpl, m_childrenImpl;
		Set m_parents, m_children;

		static void deleteDangling(std::queue<Node*>& _dangling, bool _descending);
		bool onAttach(Node& _node, bool _descending);
		bool onDetach(Node& _node, bool _deleteDangling, bool _descending);
		bool onDetachAll(bool _deleteDangling, bool _descending);

		bool onParentAttach(Node& _parent);
		bool onParentDetach(Node& _parent, bool _deleteDangling);
		bool onParentsDetachAll(bool _deleteDangling);
		bool onChildAttach(Node& _child);
		bool onChildDetach(Node& _child, bool _deleteDangling);
		bool onChildrenDetachAll(bool _deleteDangling);

	protected:

		Node(EType _type);

		virtual void onParentAttaching(Node& _parent) const;
		virtual void onChildAttaching(Node& _child) const;

		Internal::NodeSetHandle& parentsHandle();
		Internal::NodeSetHandle& childrenHandle();

	public:

		virtual ~Node();

		EType type() const;

		bool isElement() const;
		bool isOperation() const;

		bool isRoot() const;
		bool isLeaf() const;

		Element& element();
		const Element& element() const;
		Operation& operation();
		const Operation& operation() const;

		Set& forward(bool _descending);
		const Set& forward(bool _descending) const;
		Set& back(bool _descending);
		const Set& back(bool _descending) const;

		Set& parents();
		Set& children();
		const Set& parents() const;
		const Set& children() const;

	};

}