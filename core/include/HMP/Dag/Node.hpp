#pragma once

#include <HMP/Utils/SetView.hpp>

namespace HMP::Dag
{

	class Element;
	class Operation;

	class Node
	{

	public:

		using SetView = HMP::Utils::SetView<Node>;

		enum class EType
		{
			Element, Operation
		};

	private:

		const EType m_type;
		SetView m_parents{};
		SetView m_children{};

		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

	protected:

		explicit Node(EType _type);

	public:

		virtual ~Node();

		EType type() const;
		bool isElement() const;
		bool isOperation() const;

		Element& element();
		const Element& element() const;
		Operation& operation();
		const Operation& operation() const;

		SetView& parents();
		const SetView& parents() const;
		SetView& children();
		const SetView& children() const;

		bool isRoot() const;
		bool isLeaf() const;

		void attachParent(Node& _parent);
		void detachParent(Node& _parent);
		void detachParents();
		void attachChild(Node& _child);
		void detachChild(Node& _child, bool _deleteOrphaned = true);
		void detachChildren(bool _deleteOrphaned = true);
		void detachAll(bool _deleteOrphaned = true);

	};

}