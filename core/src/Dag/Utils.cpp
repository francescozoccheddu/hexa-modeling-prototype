#include <HMP/Dag/Utils.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Utils/stream.hpp>
#include <deque>
#include <unordered_map>

namespace HMP::Dag::Utils
{

	using HMP::Utils::SetView;

	SetView<Node> descendants(Node& _node, std::function<bool(const Node&)> _branchSelector)
	{
		SetView<Node> nodes{};
		std::deque<Node*> toVisit{};
		if (_branchSelector(_node))
		{
			nodes.add(_node);
			toVisit.push_back(&_node);
		}
		while (!toVisit.empty())
		{
			Node& node{ *toVisit.front() };
			toVisit.pop_front();
			for (Node& child : node.children())
			{
				if (_branchSelector(child) && nodes.add(child))
				{
					toVisit.push_back(&child);
				}
			}
		}
		return nodes;
	}

	SetView<Node, const Node> descendants(const Node& _node, std::function<bool(const Node&)> _branchSelector)
	{
		return descendants(const_cast<Node&>(_node), _branchSelector).view<const Node>();
	}

	std::ostream& operator<<(std::ostream& _stream, const Node& _node)
	{
		using HMP::Utils::operator<<;
		constexpr char sep = ' ';
		std::unordered_map<const Node*, std::size_t> nodeMap{};
		{
			SetView<Node, const Node> nodes{ descendants(_node) };
			nodeMap.reserve(nodes.size());
			unsigned int i{ 0 };
			for (const Node& node : nodes)
			{
				nodeMap.emplace(&node, i++);
			}
		}
		_stream << nodeMap.size() << sep;
		for (const auto [node, i] : nodeMap)
		{
			_stream << node->type() << sep;
			switch (node->type())
			{
				case Node::EType::Element:
				{
					const Element& element{ node->element() };
					// TODO pid
					for (const cinolib::vec3d& vertex : element.vertices())
					{
						_stream << vertex << sep;
					}
				}
				break;
				case Node::EType::Operation:
				{
					const Operation& operation{ node->operation() };
					_stream << operation.primitive() << sep;
					// TODO dependencies
					switch (operation.primitive())
					{
						case Operation::EPrimitive::Delete:
						{
							const Delete& deleteOperation{ static_cast<const Delete&>(operation) };
						}
						break;
						case Operation::EPrimitive::Extrude:
						{
							const Extrude& extrudeOperation{ static_cast<const Extrude&>(operation) };
							_stream << extrudeOperation.offset() << sep;
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							const Refine& refineOperation{ static_cast<const Refine&>(operation) };
							_stream
								<< refineOperation.needsTopologyFix() << sep
								<< refineOperation.scheme() << sep;
							for (unsigned int vid : refineOperation.vertices())
							{
								_stream << vid << sep;
							}
						}
						break;
					}
					_stream << operation.userDefined() << sep;
				}
				break;
			}
		}
		for (const auto [node, i] : nodeMap)
		{
			_stream << node->parents().size() << sep;
			for (const Node& parent : node->parents())
			{
				_stream << nodeMap.at(&parent) << sep;
			}
		}
		return _stream;
	}

	std::istream& operator>>(std::istream& _stream, Node*& _node)
	{
		using HMP::Utils::operator>>;
		std::vector<Node*> nodes{};
		std::size_t nodesCount{};
		_stream >> nodesCount;
		nodes.reserve(nodesCount);
		for (std::size_t i{ 0 }; i < nodesCount; i++)
		{
			Node::EType nodeType;
			_stream >> nodeType;
			switch (nodeType)
			{
				case Node::EType::Element:
				{
					Element& element{ *new Element };
					// TODO pid
					for (cinolib::vec3d& vertex : element.vertices())
					{
						_stream >> vertex;
					}
					nodes.push_back(&element);
				}
				break;
				case Node::EType::Operation:
				{
					Operation::EPrimitive primitive;
					Operation* operation{};
					_stream >> primitive;
					// TODO dependencies
					switch (primitive)
					{
						case Operation::EPrimitive::Delete:
						{
							Delete& deleteOperation{ *new Delete };
							operation = &deleteOperation;
						}
						break;
						case Operation::EPrimitive::Extrude:
						{
							Extrude& extrudeOperation{ *new Extrude };
							_stream >> extrudeOperation.offset();
							operation = &extrudeOperation;
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							Refine& refineOperation{ *new Refine };
							_stream
								>> refineOperation.needsTopologyFix()
								>> refineOperation.scheme();
							for (unsigned int& vid : refineOperation.vertices())
							{
								_stream >> vid;
							}
							operation = &refineOperation;
						}
						break;
					}
					_stream >> operation->userDefined();
					nodes.push_back(operation);
				}
				break;
			}
		}
		for (Node* node : nodes)
		{
			std::size_t parentsCount;
			_stream >> parentsCount;
			for (std::size_t i{ 0 }; i < parentsCount; i++)
			{
				std::size_t parentIndex;
				_stream >> parentIndex;
				node->attachParent(*nodes[parentIndex]);
			}
		}
		_node = nodes.empty() ? nullptr : nodes[0];
		return _stream;
	}

}