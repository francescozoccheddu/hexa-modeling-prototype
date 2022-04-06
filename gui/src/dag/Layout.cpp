#include <hexa-modeling-prototype/gui/dag/Layout.hpp>

#include <utility>
#include <algorithm>
#include <stdexcept>

namespace HMP::Gui::Dag
{

	// Layout::Node

	// Constructors

	Layout::Node::Node(EType _type, const std::string& _text, const Point& _center)
		: m_type{ _type }, m_text{ _text }, m_center(_center)
	{}

	Layout::Node::Node(EType _type, std::string&& _text, Point&& _center)
		: m_type{ _type }, m_text{ std::move(_text) }, m_center(std::move(_center))
	{}

	// Getters

	Layout::Node::EType Layout::Node::type() const
	{
		return m_type;
	}

	const std::string& Layout::Node::text() const
	{
		return m_text;
	}

	const Layout::Point& Layout::Node::center() const
	{
		return m_center;
	}

	// Validation

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

	// Bounding box calculation

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

	// Constructors

	Layout::Layout(const std::vector<Line>& _lines, const std::vector<Node>& _nodes, Real _lineThickness, Real _nodeRadius)
		: m_lines{ _lines }, m_nodes{ _nodes }, m_lineThickness{ _lineThickness }, m_nodeRadius{ _nodeRadius }
	{
		calculateBoundingBox();
	}

	Layout::Layout(std::vector<Line>&& _lines, std::vector<Node>&& _nodes, Real _lineThickness, Real _nodeRadius)
		: m_lines{ std::move(_lines) }, m_nodes{ std::move(_nodes) }, m_lineThickness{ _lineThickness }, m_nodeRadius{ _nodeRadius }
	{
		calculateBoundingBox();
	}

	// Getters

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