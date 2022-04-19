#pragma once

#include <vector>
#include <span>

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
		std::vector<Node*> m_parents{};
		std::vector<Node*> m_children{};

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

		std::vector<Node*>& parents();
		std::span<const Node* const> parents() const;
		std::vector<Node*>& children();
		std::span<const Node* const> children() const;

		bool isRoot() const;
		bool isLeaf() const;

	};

}