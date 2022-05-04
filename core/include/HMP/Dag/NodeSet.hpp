#pragma once

#include <cpputils/collections/DereferenceIterable.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <unordered_set>
#include <type_traits>

namespace HMP::Dag
{

	class Node;

	template<typename>
	class NodeSet;

	namespace Internal
	{

		template<typename>
		class NodeSetBase;

		class NodeSetData final : public cpputils::mixins::ReferenceClass
		{

		private:

			template<typename TNode>
			friend class NodeSetBase;

			std::unordered_set<Node*> m_set;

		public:

			bool add(Node& _node);
			bool remove(Node& _node);
			bool clear();

		};

		class NodeSetHandle final : public cpputils::mixins::ReferenceClass
		{

		private:

			template<typename>
			friend class NodeSetBase;

			template<typename>
			friend class Dag::NodeSet;

			const std::function<bool(Node&)> m_onAttach;
			const std::function<bool(Node&, bool)> m_onDetach;
			const std::function<bool(bool)> m_onDetachAll;
			NodeSetData& m_data;

			NodeSetHandle(Internal::NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll);

		};

		template<typename TNode>
		class NodeSetBase : public cpputils::mixins::ReferenceClass, public cpputils::collections::DereferenceIterable<std::unordered_set<Node*>, TNode&, const TNode&>
		{

		private:

			template<typename>
			friend class Dag::NodeSet;

			NodeSetHandle& m_handle;
			const bool m_owner;

			NodeSetBase(NodeSetHandle& _NodeSetHandle, bool _owner);

			~NodeSetBase();

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

	template<typename TNode>
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