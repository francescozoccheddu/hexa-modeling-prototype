#pragma once

#include <HMP/Utils/SetView.hpp>

namespace HMP::Dag
{

	class Element;
	class Operation;

	class Node
	{

	public:

		enum class EType
		{
			Element, Operation
		};

	private:

		const EType m_type;
		Utils::SetView<Node> m_parents{};
		Utils::SetView<Node> m_children{};

		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

	protected:

		Node(EType _type);

	public:

		virtual ~Node() = default;

		EType type() const;
		bool isElement() const;
		bool isOperation() const;

		Element& element();
		const Element& element() const;
		Operation& operation();
		const Operation& operation() const;

		Utils::SetView<Node>& parents();
		const Utils::SetView<Node>& parents() const;
		Utils::SetView<Node>& children();
		const Utils::SetView<Node>& children() const;

		bool isRoot() const;
		bool isLeaf() const;

	};

}