#pragma once

#include <HMP/Dag/Node.hpp>
#include <HMP/Utils/SetView.hpp>
#include <array>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Dag
{

	class Element : public Node
	{

	private:

		unsigned int m_pid{};
		std::array<cinolib::vec3d, 8> m_vertices;

		Utils::SetView<Operation> m_parents{ parents().view<Operation>() };
		Utils::SetView<Operation> m_children{ children().view<Operation>() };

	public:

		Element();

		unsigned int& pid();
		unsigned int pid() const;

		std::array<cinolib::vec3d, 8>& vertices();
		const std::array<cinolib::vec3d, 8>& vertices() const;

		const Operation& parent() const;

		Utils::SetView<Operation>& parents();
		const Utils::SetView<Operation>& parents() const;
		Utils::SetView<Operation>& children();
		const Utils::SetView<Operation>& children() const;

	};

}