#include <hexa-modeling-prototype/gui/dag/Layout.hpp>

#include <utility>
#include <algorithm>
#include <stdexcept>

namespace HMP::Gui::Dag
{

	// Layout::Node

	Layout::Node::Node(const Point& _center, HMP::NodeType _type, NodeData _data)
		: m_center(_center ), m_type{_type}, m_data{_data}
	{}

	Layout::Node Layout::Node::element(const Point& _center, unsigned int _id)
	{
		return Node{ _center, HMP::NodeType::ELEMENT, {.m_elementId{_id}} };
	}

	Layout::Node Layout::Node::operation(const Point& _center, HMP::Primitive _primitive)
	{
		return Node{ _center, HMP::NodeType::OPERATION, {.m_operationPrimitive{_primitive}} };
	}

	HMP::NodeType Layout::Node::type() const
	{
		return m_type;
	}

	unsigned int Layout::Node::elementId() const
	{
		if (m_type != HMP::NodeType::ELEMENT)
		{
			throw std::logic_error{ "not an element node" };
		}
		return m_data.m_elementId;
	}

	HMP::Primitive Layout::Node::operationPrimitive() const
	{
		if (m_type != HMP::NodeType::OPERATION)
		{
			throw std::logic_error{ "not an operation node" };
		}
		return m_data.m_operationPrimitive;
	}

	const Layout::Point& Layout::Node::center() const
	{
		return m_center;
	}

	// Layout

	void Layout::validate() const
	{
		if (m_nodeRadius <= 0.f)
		{
			throw std::domain_error{ "node radius must be positive" };
		}
		if (m_lineThickness <= 0.f)
		{
			throw std::domain_error{ "line thickness must be positive" };
		}
	}

	void Layout::calculateBoundingBox()
	{
		const Real halfLineThickness = m_lineThickness / 2;
		for (const Line& line : m_lines)
		{
			expandBoundingBox(line.first, halfLineThickness);
			expandBoundingBox(line.second, halfLineThickness);
		}
		for (const Node& node : m_nodes)
		{
			expandBoundingBox(node.center(), m_nodeRadius);
		}
	}

	void Layout::expandBoundingBox(const Point& _center, Real _extent)
	{
		m_boundingBox.first.x() = std::min(m_boundingBox.first.x(), _center.x() - _extent);
		m_boundingBox.first.y() = std::min(m_boundingBox.first.y(), _center.y() - _extent);
		m_boundingBox.second.x() = std::min(m_boundingBox.second.x(), _center.x() + _extent);
		m_boundingBox.second.y() = std::min(m_boundingBox.second.y(), _center.y() + _extent);
	}

	Layout::Layout(const std::vector<Node>& _nodes, const std::vector<Line>& _lines, Real _nodeRadius, Real _lineThickness)
		: m_nodes{ _nodes }, m_lines{ _lines }, m_lineThickness{ _lineThickness }, m_nodeRadius{ _nodeRadius }
	{
		calculateBoundingBox();
	}

	Layout::Layout(std::vector<Node>&& _nodes, std::vector<Line>&& _lines, Real _nodeRadius, Real _lineThickness)
		: m_nodes{ std::move(_nodes) }, m_lines{ std::move(_lines) }, m_lineThickness{ _lineThickness }, m_nodeRadius{ _nodeRadius }
	{
		calculateBoundingBox();
	}

	const std::vector<Layout::Line>& Layout::lines() const
	{
		return m_lines;
	}

	const std::vector<Layout::Node>& Layout::nodes() const
	{
		return m_nodes;
	}

	const Layout::Line& Layout::boundingBox() const
	{
		return m_boundingBox;
	}

	Layout::Real Layout::lineThickness() const
	{
		return m_lineThickness;
	}

	Layout::Real Layout::nodeRadius() const
	{
		return m_nodeRadius;
	}

}