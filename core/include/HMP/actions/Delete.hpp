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
		
		Delete(const cinolib::vec3d& _polyCentroid);

		void apply() override;
		void unapply() override;

	private:

		const cinolib::vec3d m_polyCentroid;
		Dag::Delete* m_operation{};

	};

}