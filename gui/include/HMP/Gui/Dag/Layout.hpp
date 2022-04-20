#pragma once

#include <utility>
#include <vector>
#include <string>
#include <cmath>
#include <HMP/Dag/Node.hpp>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Gui::Dag
{

	class Layout final
	{

	public:

		using Real = double;
		using Point = cinolib::vec<2, Real>;
		using Line = std::pair<Point, Point>;

		class Node final
		{

		private:

			const HMP::Dag::Node* m_node{};
			Point m_center{};


		public:

			Node(const Point& _center, const HMP::Dag::Node& _node);

			const HMP::Dag::Node& node() const;
			const Point& center() const;

		};

	private:

		std::vector<Line> m_lines;
		std::vector<Node> m_nodes;
		Point m_bottomLeft;
		Point m_topRight;
		Point m_size;
		Real m_aspectRatio;
		Real m_lineThickness;
		Real m_nodeRadius;

		void validate() const;
		void calculateBoundingBox();
		void expandBoundingBox(const Point& _center, Real _extent);

	public:

		Layout();
		Layout(const std::vector<Node>& _nodes, const std::vector<Line>& _lines, Real _lineThickness, Real _nodeRadius);
		Layout(std::vector<Node>&& _nodes, std::vector<Line>&& _lines, Real _lineThickness, Real _nodeRadius);

		const std::vector<Line>& lines() const;
		const std::vector<Node>& nodes() const;
		const Point& bottomLeft() const;
		const Point& topRight() const;
		const Point& size() const;
		double aspectRatio() const;
		Real lineThickness() const;
		Real nodeRadius() const;

	};

}