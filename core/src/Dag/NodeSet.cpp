#include <HMP/Dag/NodeSet.hpp>

namespace HMP::Dag
{

	namespace Internal
	{

		// NodeSetData

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

		// NodeSetHandle

		NodeSetHandle::NodeSetHandle(NodeSetData& _data, std::function<bool(Node&)> _onAttach, std::function<bool(Node&, bool)> _onDetach, std::function<bool(bool)> _onDetachAll)
			: m_data{ _data }, m_onAttach{ _onAttach }, m_onDetach{ _onDetach }, m_onDetachAll{ _onDetachAll }
		{}
		
	}

}