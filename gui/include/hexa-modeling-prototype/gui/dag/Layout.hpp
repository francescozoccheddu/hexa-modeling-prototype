#pragma once

#include <utility>
#include <vector>
#include <string>
#include <cmath>
#include <hexa-modeling-prototype/operationstree.hpp>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Gui::Dag
{

	class Layout final
	{

	public:

		using Real = double;
		using Point = cinolib::vec<2, Real>;
		using Line = std::pair<Point, Point>;
		using Box = std::pair<Point, Point>;

		class Node final
		{

		private:

			HMP::NodeType m_type;

			union NodeData
			{
				HMP::Primitive m_operationPrimitive;
				unsigned int m_elementId;
			} m_data;

			Point m_center{};

			Node(const Point& _center, HMP::NodeType _type, NodeData _data);

		public:

			static Node element(const Point& _center, unsigned int _id);
			static Node operation(const Point& _center, HMP::Primitive _primitive);

			unsigned int elementId() const;
			HMP::Primitive operationPrimitive() const;
			HMP::NodeType type() const;
			const Point& center() const;

		};

	private:

		std::vector<Line> m_lines{};
		std::vector<Node> m_nodes{};
		Box m_boundingBox{};
		Real m_lineThickness{};
		Real m_nodeRadius{};

		void validate() const;
		void calculateBoundingBox();
		void expandBoundingBox(const Point& _center, Real _extent);

	public:

		Layout(const std::vector<Node>& _nodes, const std::vector<Line>& _lines,  Real _lineThickness, Real _nodeRadius);
		Layout(std::vector<Node>&& _nodes, std::vector<Line>&& _lines,  Real _lineThickness, Real _nodeRadius);

		const std::vector<Line>& lines() const;
		const std::vector<Node>& nodes() const;
		const Box& boundingBox() const;
		Real lineThickness() const;
		Real nodeRadius() const;

	};

}