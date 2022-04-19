#pragma once

#include <HMP/Dag/Node.hpp>
#include <HMP/Utils/SetView.hpp>
#include <array>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Dag
{

	class Element : public Node
	{

	public:

		using DagSetView = HMP::Utils::SetView<Node, Operation>;

	private:

		unsigned int m_pid{};
		std::array<cinolib::vec3d, 8> m_vertices;

		DagSetView m_parents{ Node::parents().view<Operation>() };
		DagSetView m_children{ Node::children().view<Operation>() };

	public:

		Element();

		unsigned int& pid();
		unsigned int pid() const;

		std::array<cinolib::vec3d, 8>& vertices();
		const std::array<cinolib::vec3d, 8>& vertices() const;

		const Operation& parent() const;

		DagSetView& parents();
		const DagSetView& parents() const;
		DagSetView& children();
		const DagSetView& children() const;

	};

}