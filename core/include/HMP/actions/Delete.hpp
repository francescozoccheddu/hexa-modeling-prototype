#pragma once

#include <array>
#include <cinolib/geometry/vec_mat.h>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class Delete final : public Commander::Action
	{

	public:
		
		Delete(unsigned int _pid);

		void apply() override;
		void unapply() override;

	private:

		unsigned int m_pid;
		std::array<cinolib::vec3d, 8> m_vertices;
		Dag::Delete* m_operation{};

	};

}