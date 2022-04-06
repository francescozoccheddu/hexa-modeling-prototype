#pragma once

#include <utility>
#include <vector>
#include <string>
#include <cmath>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Gui::Dag
{

	class Layout final
	{

	public:

		using Real = double;
		using Point = cinolib::mat<2, 1, Real>;
		using Line = std::pair<Point, Point>;

		class Node final
		{

		public:

			enum class EType
			{
				Operation, Hexagon
			};

		private:

			EType m_type;
			std::string m_text;
			Point m_center{};

		public:

			Node(EType _type, const std::string& _text, const Point& _center);
			Node(EType _type, std::string&& _text, Point&& _center);

			EType type() const;
			const std::string& text() const;
			const Point& center() const;

		};

	private:

		std::vector<Line> m_lines{};
		std::vector<Node> m_nodes{};
		Line m_boundingBox{};
		Real m_lineThickness{};
		Real m_nodeRadius{};

		void validate() const;
		void calculateBoundingBox();
		void expandBoundingBox(const Point& _center, Real _extent);

	public:

		Layout(const std::vector<Line>& _lines, const std::vector<Node>& _nodes, Real _lineThickness, Real _nodeRadius);
		Layout(std::vector<Line>&& _lines, std::vector<Node>&& _nodes, Real _lineThickness, Real _nodeRadius);

		const std::vector<Line>& lines() const;
		const std::vector<Node>& nodes() const;
		const Line& boundingBox() const;
		Real lineThickness() const;
		Real nodeRadius() const;

	};

}