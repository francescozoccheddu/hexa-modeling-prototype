#ifndef HMP_DAG_NODESET_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Dag/NodeSet.hpp>

#include <stdexcept>

namespace HMP::Dag
{

	namespace Internal
	{

		// NodeSetBase

		template<typename TNode>
		NodeSetBase<TNode>::NodeSetBase(NodeSetHandle& _handle, bool _owner)
			: m_handle{ _handle }, m_owner{ _owner }, cpputils::collections::DereferenceIterable<std::unordered_set<Node*>, TNode&, const TNode&>{ _handle.m_data.m_set }
		{}

		template<typename TNode>
		NodeSetBase<TNode>::~NodeSetBase()
		{
			if (m_owner)
			{
				delete& m_handle;
			}
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::attach(TNode& _node)
		{
			return m_handle.m_onAttach(_node);
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::detach(TNode& _node, bool _deleteDangling)
		{
			return m_handle.m_onDetach(_node, _deleteDangling);
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::detachAll(bool _deleteDangling)
		{
			return m_handle.m_onDetachAll(_deleteDangling);
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::has(const TNode& _node) const
		{
			return m_handle.m_data.m_set.contains(_node);
		}

		template<typename TNode>
		std::size_t NodeSetBase<TNode>::size() const
		{
			return m_handle.m_data.m_set.size();
		}

		template<typename TNode>
		bool NodeSetBase<TNode>::empty() const
		{
			return m_handle.m_data.m_set.empty();
		}

		template<typename TNode>
		TNode& NodeSetBase<TNode>::single()
		{
			if (size() != 1)
			{
				throw std::logic_error{ "not a singleton" };
			}
			return *reinterpret_cast<TNode*>(*m_handle.m_data.m_set.begin());
		}

		template<typename TNode>
		const TNode& NodeSetBase<TNode>::single() const
		{
			return const_cast<NodeSetBase*>(this)->single();
		}

	}

	// NodeSet

	template<typename TNode>
	NodeSet<TNode>::NodeSet(Internal::NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll)
		: Internal::NodeSetBase<TNode>{ *new Internal::NodeSetHandle{ _data, _onAttach, _onDetach, _onDetachAll }, true }
	{}

	template<typename TNode>
	Internal::NodeSetHandle& NodeSet<TNode>::handle()
	{
		return Internal::NodeSetBase<TNode>::m_handle;
	}

	template<typename TNode>
	Internal::NodeSetData& NodeSet<TNode>::data()
	{
		return Internal::NodeSetBase<TNode>::m_handle.m_data;
	}

	template<typename TNode>
	const Internal::NodeSetData& NodeSet<TNode>::data() const
	{
		return Internal::NodeSetBase<TNode>::m_handle.m_data;
	}

	template<typename TNode>
	NodeSet<TNode>::NodeSet(Internal::NodeSetHandle& _handle)
		: Internal::NodeSetBase<TNode>{ _handle, false }
	{}

}