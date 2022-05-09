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

	void serialize(HMP::Utils::Serialization::Serializer& _serializer, const Node& _node)
	{
		const std::vector<const Node*> nodes{ descendants(_node) };
		_serializer << nodes.size();
		for (const Node* node : nodes)
		{
			_serializer << node->type();
			switch (node->type())
			{
				case Node::EType::Element:
				{
					const Element& element{ node->element() };
					for (const Vec& vertex : element.vertices())
					{
						_serializer << vertex;
					}
				}
				break;
				case Node::EType::Operation:
				{
					const Operation& operation{ node->operation() };
					_serializer << operation.primitive();
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
							_serializer << extrudeOperation.faceOffset();
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							const Refine& refineOperation{ static_cast<const Refine&>(operation) };
							_serializer
								<< refineOperation.forwardFaceOffset()
								<< refineOperation.upFaceOffset()
								<< refineOperation.scheme();
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
				_serializer << node->parents().size();
				for (const Node& parent : node->parents())
				{
					_serializer << nodeMap.at(&parent);
				}
			}
		}
	}

	Node& deserialize(HMP::Utils::Serialization::Deserializer& _deserializer)
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
								>> refineOperation.forwardFaceOffset()
								>> refineOperation.upFaceOffset()
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
		if (nodes.empty())
		{
			throw std::logic_error{ "empty" };
		}
		return *nodes[0];
	}

}