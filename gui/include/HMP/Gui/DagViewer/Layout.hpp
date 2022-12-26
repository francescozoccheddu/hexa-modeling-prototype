#pragma once

#include <utility>
#include <vector>
#include <string>
#include <cmath>
#include <HMP/Dag/Node.hpp>
#include <HMP/Meshing/types.hpp>

namespace HMP::Gui::DagViewer
{

	class Layout final
	{

	public:

		class Node final
		{

		private:

			const Dag::Node* m_node{};
			Vec2 m_center{};


		public:

			Node(const Vec2& _center, const Dag::Node& _node);

			const Dag::Node& node() const;
			const Vec2& center() const;

		};

	private:

		std::vector<std::pair<Vec2, Vec2>> m_lines;
		std::vector<Node> m_nodes;
		Vec2 m_bottomLeft;
		Vec2 m_topRight;
		Vec2 m_size;
		Real m_aspectRatio;
		Real m_lineThickness;
		Real m_nodeRadius;

		void calculateBoundingBox();
		void expandBoundingBox(const Vec2& _center, Real _extent);

	public:

		Layout();
		Layout(const std::vector<Node>& _nodes, const std::vector<std::pair<Vec2, Vec2>>& _lines, Real _lineThickness, Real _nodeRadius);
		Layout(std::vector<Node>&& _nodes, std::vector<std::pair<Vec2, Vec2>>&& _lines, Real _lineThickness, Real _nodeRadius);

		const std::vector<std::pair<Vec2, Vec2>>& lines() const;
		const std::vector<Node>& nodes() const;
		const Vec2& bottomLeft() const;
		const Vec2& topRight() const;
		const Vec2& size() const;
		double aspectRatio() const;
		Real lineThickness() const;
		Real nodeRadius() const;

	};

}