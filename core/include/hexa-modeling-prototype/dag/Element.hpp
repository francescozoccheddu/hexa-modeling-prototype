#pragma once

#include <hexa-modeling-prototype/dag/Node.hpp>
#include <span>
#include <array>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Dag
{

	class Element : public Node
	{

	private:

		unsigned int m_pid{};
		std::array<cinolib::vec3d, 8> m_vertices;

	public:

		Element();

		unsigned int& pid();
		unsigned int pid() const;

		std::array<cinolib::vec3d, 8>& vertices();
		const std::array<cinolib::vec3d, 8>& vertices() const;

		Operation*& parent();
		const Operation* parent() const;
		std::span<Operation*> children();
		std::span<const Operation* const> children() const;

	};

}