#include <hexa-modeling-prototype/gui/dag/createLayout.hpp>

#include <unordered_map>
#include <deque>
#include <ogdf/basic/Logger.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/layered/MedianHeuristic.h>
#include <ogdf/layered/OptimalHierarchyLayout.h>
#include <ogdf/layered/OptimalRanking.h>
#include <ogdf/layered/SugiyamaLayout.h>
#include <memory>
#include <utility>
#include <stdexcept>

namespace HMP::Gui::Dag
{

	constexpr double c_nodeRadius{ 1.0 };
	constexpr double c_lineThickness{ c_nodeRadius / 20.0 };
	constexpr double c_nodeDistance{ c_nodeRadius + 0.25 };

	std::unordered_map<const HMP::Node*, ogdf::node> populateGraph(const HMP::OperationsTree& _dag, ogdf::Graph& _graph)
	{
		std::unordered_map<const HMP::Node*, ogdf::node> dagToGraphNodeMap{};
		// nodes
		{
			std::deque<const HMP::Node*> dagNodesToVisit{ _dag.root.get() };
			while (!dagNodesToVisit.empty())
			{
				const HMP::Node* dagNode{ dagNodesToVisit.front() };
				dagNodesToVisit.pop_front();
				const std::pair<decltype(dagToGraphNodeMap)::iterator, bool> insertion{ dagToGraphNodeMap.insert({ dagNode, nullptr }) };
				if (insertion.second)
				{
					insertion.first->second = _graph.newNode();
					switch (dagNode->type)
					{
						case HMP::NodeType::ELEMENT:
							for (const std::shared_ptr<HMP::Operation> child : static_cast<const HMP::Element*>(dagNode)->operations)
							{
								dagNodesToVisit.push_back(child.get());
							}
							break;
						case HMP::NodeType::OPERATION:
							for (const std::shared_ptr<HMP::Element> child : static_cast<const HMP::Operation*>(dagNode)->children)
							{
								dagNodesToVisit.push_back(child.get());
							}
							break;
					}
				}
			}
		}
		// edges
		{
			for (auto const& [dagNode, graphNode] : dagToGraphNodeMap)
			{
				for (const std::shared_ptr<HMP::Node> dagParent : dagNode->parents)
				{
					const ogdf::node graphParent{ dagToGraphNodeMap[dagParent.get()] };
					_graph.newEdge(graphParent, graphNode);
				}
			}
		}

		return dagToGraphNodeMap;
	}

	ogdf::GraphAttributes layoutGraph(const ogdf::Graph& _graph)
	{
		ogdf::GraphAttributes graphAttributes(_graph,
			ogdf::GraphAttributes::nodeGraphics |
			ogdf::GraphAttributes::edgeGraphics
		);

		graphAttributes.setAllHeight(c_nodeRadius);
		graphAttributes.setAllWidth(c_nodeRadius);

		ogdf::SugiyamaLayout sugiyamaLayout{};
		sugiyamaLayout.setRanking(new ogdf::OptimalRanking);
		sugiyamaLayout.setCrossMin(new ogdf::MedianHeuristic);

		ogdf::OptimalHierarchyLayout* optimalHierarchyLayout{ new ogdf::OptimalHierarchyLayout };
		optimalHierarchyLayout->layerDistance(c_nodeDistance);
		optimalHierarchyLayout->nodeDistance(c_nodeDistance);
		optimalHierarchyLayout->weightBalancing(0.8);
		sugiyamaLayout.setLayout(optimalHierarchyLayout);

		sugiyamaLayout.call(graphAttributes);

		return graphAttributes;
	}

	Layout::Node createLayoutNode(const HMP::Node& _dagNode, const ogdf::node& _graphNode, const HMP::Grid& _grid, const std::unordered_map<const HMP::Element*, std::shared_ptr<HMP::Element>>& _elementsMap, const ogdf::GraphAttributes& _graphAttributes)
	{
		const Layout::Point center{ _graphAttributes.x(_graphNode), -_graphAttributes.y(_graphNode) };
		switch (_dagNode.type)
		{
			case NodeType::ELEMENT:
			{
				const HMP::Element& dagElement{ static_cast<const HMP::Element&>(_dagNode) };
				const std::vector<unsigned int> vids{ _grid.op_tree.element2vids.at(_elementsMap.at(&dagElement)) };
				return Layout::Node::element(center, _grid.vids2pid(vids));
			}
			case NodeType::OPERATION:
			{
				const HMP::Operation& dagOperation{ static_cast<const HMP::Operation&>(_dagNode) };
				return Layout::Node::operation(center, dagOperation.primitive);
			}
			default:
				throw std::domain_error{ "unknown node type" };
		}
	}

	std::vector<Layout::Node> createLayoutNodes(const HMP::Grid& _grid, const std::unordered_map<const HMP::Node*, ogdf::node>& _dagToGraphNodeMap, const ogdf::GraphAttributes& _graphAttributes)
	{
		std::vector<Layout::Node> nodes{ };
		nodes.reserve(static_cast<size_t>(_graphAttributes.constGraph().numberOfNodes()));
		std::unordered_map<const HMP::Element*, std::shared_ptr<HMP::Element>> elementsMap{}; // this is ugly
		for (auto const& [element, vids] : _grid.op_tree.element2vids)
		{
			elementsMap.emplace(element.get(), element);
		}
		for (auto const& [dagNode, graphNode] : _dagToGraphNodeMap)
		{
			nodes.push_back(createLayoutNode(*dagNode, graphNode, _grid, elementsMap, _graphAttributes));
		}
		return nodes;
	}

	std::vector<Layout::Line> createLayoutLines(const ogdf::GraphAttributes& _graphAttributes)
	{
		std::vector<Layout::Line> lines{};
		{
			size_t count{ 0 };
			for (const ogdf::edge edge : _graphAttributes.constGraph().edges)
			{
				count += _graphAttributes.bends(edge).size() + 1;
			}
			lines.reserve(count);
		}
		for (const ogdf::edge edge : _graphAttributes.constGraph().edges)
		{
			const ogdf::DPolyline& polyline{ _graphAttributes.bends(edge) };
			ogdf::DPoint lastPoint{ _graphAttributes.x(edge->source()), _graphAttributes.y(edge->source()) };
			for (const ogdf::DPoint point : polyline)
			{
				lines.push_back({ Layout::Point{lastPoint.m_x, -lastPoint.m_y}, Layout::Point{point.m_x, -point.m_y} });
				lastPoint = point;
			}
			const ogdf::DPoint endPoint{ _graphAttributes.x(edge->target()), _graphAttributes.y(edge->target()) };
			lines.push_back({ Layout::Point{lastPoint.m_x, -lastPoint.m_y}, Layout::Point{endPoint.m_x, -endPoint.m_y} });
		}
		return lines;
	}

	Layout createLayout(const HMP::Grid& _grid)
	{
		ogdf::Logger::globalLogLevel(ogdf::Logger::Level::Alarm);
		ogdf::Graph graph{};
		const std::unordered_map<const HMP::Node*, ogdf::node> dagToGraphNodeMap{ populateGraph(_grid.op_tree, graph) };
		const ogdf::GraphAttributes graphAttributes{ layoutGraph(graph) };
		std::vector<Layout::Node> nodes{ createLayoutNodes(_grid, dagToGraphNodeMap, graphAttributes) };
		std::vector<Layout::Line> lines{ createLayoutLines(graphAttributes) };
		return Layout{ std::move(nodes), std::move(lines),  c_nodeRadius, c_lineThickness };
	}

}