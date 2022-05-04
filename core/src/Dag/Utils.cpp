#include <HMP/Dag/Utils.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Utils/enums.hpp>
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

	std::ostream& operator<<(std::ostream& _stream, const Node& _node)
	{
		using HMP::Utils::operator<<;
		constexpr char sep = ' ';
		const std::vector<const Node*> nodes{ descendants(_node) };
		_stream << nodes.size() << sep;
		for (const Node* node : nodes)
		{
			_stream << node->type() << sep;
			switch (node->type())
			{
				case Node::EType::Element:
				{
					const Element& element{ node->element() };
					// TODO pid
					for (const Vec& vertex : element.vertices())
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
							_stream << extrudeOperation.faceOffset() << sep;
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							const Refine& refineOperation{ static_cast<const Refine&>(operation) };
							_stream
								<< refineOperation.needsTopologyFix() << sep
								<< refineOperation.scheme() << sep;
							for (Id vid : refineOperation.vertices())
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
				_stream << node->parents().size() << sep;
				for (const Node& parent : node->parents())
				{
					_stream << nodeMap.at(&parent) << sep;
				}
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
					Element& element{ *new Element{} };
					// TODO pid
					for (Vec& vertex : element.vertices())
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
							Delete& deleteOperation{ *new Delete{} };
							operation = &deleteOperation;
						}
						break;
						case Operation::EPrimitive::Extrude:
						{
							Extrude& extrudeOperation{ *new Extrude{} };
							_stream >> extrudeOperation.faceOffset();
							operation = &extrudeOperation;
						}
						break;
						case Operation::EPrimitive::Refine:
						{
							Refine& refineOperation{ *new Refine{} };
							_stream
								>> refineOperation.needsTopologyFix()
								>> refineOperation.scheme();
							for (Id& vid : refineOperation.vertices())
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
				node->parents().attach(*nodes[parentIndex]);
			}
		}
		_node = nodes.empty() ? nullptr : nodes[0];
		return _stream;
	}

}