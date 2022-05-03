#pragma once

#include <cpputils/collections/DereferenceIterable.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <unordered_set>
#include <type_traits>

namespace HMP::Dag
{

	class Node;

	namespace Internal
	{

		template<typename TNode> concept IsValidNodeSet
			= std::is_class_v<TNode>
			&& std::is_base_of_v<Node, TNode>
			&& (!std::is_const_v<TNode>);

		class NodeSetData final : public cpputils::mixins::ReferenceClass
		{

		private:

			template<typename TNode> requires IsValidNodeSet<TNode>
			friend class NodeSet;

			std::unordered_set<Node*> m_set;

		public:

			bool add(Node& _node);
			bool remove(Node& _node);
			bool clear();

		};

		class NodeSetHandle final : public cpputils::mixins::ReferenceClass
		{

		private:

			template<typename TNode> requires IsValidNodeSet<TNode>
			friend class NodeSetBase;

			template<typename TNode> requires IsValidNodeSet<TNode>
			friend class NodeSet;

			const std::function<bool(Node&)> m_onAttach;
			const std::function<bool(Node&, bool)> m_onDetach;
			const std::function<bool(bool)> m_onDetachAll;
			NodeSetData& m_data;

		};

		template<typename TNode> requires IsValidNodeSet<TNode>
		class NodeSetBase : public cpputils::mixins::ReferenceClass, public cpputils::collections::DereferenceIterable<std::unordered_set<Node*>, TNode>
		{

		private:

			template<typename TNode> requires IsValidNodeSet<TNode>
			friend class NodeSet;

			NodeSetHandle& m_handle;
			const bool m_owner;

			NodeSetBase(NodeSetHandle& _NodeSetHandle, bool _owner);

		public:

			bool attach(TNode& _node);
			bool detach(TNode& _node, bool _deleteDangling = false);
			bool detachAll(bool _deleteDangling = false);

			bool has(const TNode& _node) const;
			std::size_t size() const;
			bool empty() const;
			TNode& single();
			const TNode& single() const;

		};

	}

	template<typename TNode> requires Internal::IsValidNodeSet<TNode>
	class NodeSet final : public Internal::NodeSetBase<TNode>
	{

	private:

		friend class Node;

		NodeSet(Internal::NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll);

		Internal::NodeSetHandle& handle();
		Internal::NodeSetData& data();
		const Internal::NodeSetData& data() const;

	public:

		NodeSet(Internal::NodeSetHandle& _NodeSetHandle);

	};

}

#define HMP_DAG_NODESET_IMPL
#include <HMP/Dag/NodeSet.tpp>
#undef HMP_DAG_NODESET_IMPL