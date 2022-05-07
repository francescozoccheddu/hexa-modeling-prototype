#include <HMP/Dag/Utils.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <HMP/Dag/Element.hpp>
#include <deque>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace HMP::Dag::Utils
{

	std::vector<Node*> descendants(Node& _node, std::function<bool(const Node&)> _branchSelector)
	{
		std::vector<Node*> nodes{};
		std::unordered_set<Node*> visited{};
		std::deque<Node*> toVisit{};
		if (_branchSelector(_node))
		{
			nodes.push_back(&_node);
			visited.emplace(&_node);
			toVisit.push_back(&_node);
		}
		while (!toVisit.empty())
		{
			Node& node{ *toVisit.front() };
			toVisit.pop_front();
			for (Node& child : node.children())
			{
				if (_branchSelector(child) && visited.emplace(&child).second)
				{
					nodes.push_back(&child);
					toVisit.push_back(&child);
				}
			}
		}
		return nodes;
	}

	std::vector<const Node*> descendants(const Node& _node, std::function<bool(const Node&)> _branchSelector)
	{
		std::vector<Node*> result{ descendants(const_cast<Node&>(_node), _branchSelector) };
		return std::vector<const Node*>{&result[0], (&result[0]) + result.size()};
	}

	HMP::Utils::Serialization::Serializer& operator<<(HMP::Utils::Serialization::Serializer& _serializer, const Node& _node)
	{
		constexpr char sep = ' ';
		const std::vector<const Node*> nodes{ descendants(_node) };
		_serializer << nodes.size() << sep;
		for (const Node* node : nodes)
		{
			_serializer << node->type() << sep;
			switch (node->type())
			{
				case Node::EType::Element:
				{
					const Element& element{ node->element() };
					// TODO pid
					for (const Vec& vertex : element.vertices())
					{
						_serializer << vertex << sep;
					}
				}
				break;
				case Node::EType::Operation:
				{
					const Operation& operation{ node->operation() };
					_serializer << operation.primitive() << sep;
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
							_serializer << extrudeOperation.faceOffset() << sep;
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							const Refine& refineOperation{ static_cast<const Refine&>(operation) };
							_serializer
								<< refineOperation.faceOffset() << sep
								<< refineOperation.scheme() << sep;
						}
						break;
					}
				}
				break;
			}
		}
		{
			std::unordered_map<const Node*, std::size_t> nodeMap{};
			{
				nodeMap.reserve(nodes.size());
				Id i{ 0 };
				for (const Node* node : nodes)
				{
					nodeMap.emplace(node, i++);
				}
			}
			for (const Node* node : nodes)
			{
				_serializer << node->parents().size() << sep;
				for (const Node& parent : node->parents())
				{
					_serializer << nodeMap.at(&parent) << sep;
				}
			}
		}
		return _serializer;
	}

	HMP::Utils::Serialization::Deserializer& operator>>(HMP::Utils::Serialization::Deserializer& _deserializer, Node*& _node)
	{
		std::vector<Node*> nodes{};
		std::size_t nodesCount{};
		_deserializer >> nodesCount;
		nodes.reserve(nodesCount);
		for (std::size_t i{ 0 }; i < nodesCount; i++)
		{
			Node::EType nodeType;
			_deserializer >> nodeType;
			switch (nodeType)
			{
				case Node::EType::Element:
				{
					Element& element{ *new Element{} };
					// TODO pid
					for (Vec& vertex : element.vertices())
					{
						_deserializer >> vertex;
					}
					nodes.push_back(&element);
				}
				break;
				case Node::EType::Operation:
				{
					Operation::EPrimitive primitive;
					Operation* operation{};
					_deserializer >> primitive;
					// TODO dependencies
					switch (primitive)
					{
						case Operation::EPrimitive::Delete:
						{
							Delete& deleteOperation{ *new Delete{} };
							operation = &deleteOperation;
						}
						break;
						case Operation::EPrimitive::Extrude:
						{
							Extrude& extrudeOperation{ *new Extrude{} };
							_deserializer >> extrudeOperation.faceOffset();
							operation = &extrudeOperation;
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							Refine& refineOperation{ *new Refine{} };
							_deserializer
								>> refineOperation.faceOffset()
								>> refineOperation.scheme();
							operation = &refineOperation;
						}
						break;
					}
					nodes.push_back(operation);
				}
				break;
			}
		}
		for (Node* node : nodes)
		{
			std::size_t parentsCount;
			_deserializer >> parentsCount;
			for (std::size_t i{ 0 }; i < parentsCount; i++)
			{
				std::size_t parentIndex;
				_deserializer >> parentIndex;
				node->parents().attach(*nodes[parentIndex]);
			}
		}
		_node = nodes.empty() ? nullptr : nodes[0];
		return _deserializer;
	}

}