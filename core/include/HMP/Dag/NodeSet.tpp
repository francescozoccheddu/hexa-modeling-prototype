#ifndef HMP_DAG_NODESET_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Dag/NodeSet.hpp>

#include <stdexcept>

namespace HMP::Dag
{

	namespace Internal
	{

		bool NodeSetData::add(Node& _node)
		{
			return m_set.insert(&_node).second;
		}

		bool NodeSetData::remove(Node& _node)
		{
			return m_set.erase(&_node);
		}

		bool NodeSetData::clear()
		{
			const bool wasEmpty{ m_set.empty() };
			m_set.clear();
			return !wasEmpty;
		}

		template<typename TNode> requires IsValidNodeSet<TNode>
		NodeSetBase<TNode>::NodeSetBase(NodeSetHandle& _handle, bool _owner)
			: m_handle{ _handle }, m_owner{ _owner }, cpputils::collections::DereferenceIterable<std::unordered_set<Node*>, TNode>{ _handle.m_data.m_set }
		{}

		template<typename TNode> requires IsValidNodeSet<TNode>
		bool NodeSetBase<TNode>::attach(TNode& _node)
		{
			return m_handle.m_onAttach(_node);
		}

		template<typename TNode> requires IsValidNodeSet<TNode>
		bool NodeSetBase<TNode>::detach(TNode& _node, bool _deleteDangling)
		{
			return m_handle.m_onDetach(_node, _deleteDangling);
		}

		template<typename TNode> requires IsValidNodeSet<TNode>
		bool NodeSetBase<TNode>::detachAll(bool _deleteDangling)
		{
			return m_handle.m_onDetachAll(_deleteDangling);
		}

		template<typename TNode> requires IsValidNodeSet<TNode>
		bool NodeSetBase<TNode>::has(const TNode& _node) const
		{
			return m_handle.m_data.m_set.contains(_node);
		}

		template<typename TNode> requires IsValidNodeSet<TNode>
		std::size_t NodeSetBase<TNode>::size() const
		{
			return m_handle.m_data.m_set.size();
		}

		template<typename TNode> requires IsValidNodeSet<TNode>
		bool NodeSetBase<TNode>::empty() const
		{
			return m_handle.m_data.m_set.empty();
		}

		template<typename TNode> requires IsValidNodeSet<TNode>
		TNode& NodeSetBase<TNode>::single()
		{
			if (size() != 1)
			{
				throw std::logic_error{ "not a singleton" };
			}
			return *m_handle.m_data.m_set.begin();
		}

		template<typename TNode> requires IsValidNodeSet<TNode>
		const TNode& NodeSetBase<TNode>::single() const
		{
			return const_cast<NodeSetBase*>(this)->single();
		}

	}

	template<typename TNode> requires Internal::IsValidNodeSet<TNode>
	NodeSet<TNode>::NodeSet(Internal::NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll)
		: NodeSetBase{ Internal::NodeSetHandle{.m_data{_data}, .m_onAttach{_onAttach}, .m_onDetach{_onDetach}, .m_onDetachAll{_onDetachAll} }, true }
	{}

	template<typename TNode> requires Internal::IsValidNodeSet<TNode>
	Internal::NodeSetHandle& NodeSet<TNode>::handle()
	{
		return m_handle;
	}

	template<typename TNode> requires Internal::IsValidNodeSet<TNode>
	Internal::NodeSetData& NodeSet<TNode>::data()
	{
		return m_handle.m_data;
	}

	template<typename TNode> requires Internal::IsValidNodeSet<TNode>
	const Internal::NodeSetData& NodeSet<TNode>::data() const
	{
		return m_handle.m_data;
	}

	template<typename TNode> requires Internal::IsValidNodeSet<TNode>
	NodeSet<TNode>::NodeSet(Internal::NodeSetHandle& _handle)
		: NodeSetBase{ _handle, false }
	{}

}