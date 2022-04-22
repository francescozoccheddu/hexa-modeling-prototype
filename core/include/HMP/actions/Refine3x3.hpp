#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/Refine.hpp>
#include <cinolib/geometry/vec_mat.h>

namespace HMP::Actions
{

	class Refine3x3 final : public Commander::Action
	{

	public:

		Refine3x3(const cinolib::vec3d& _polyCentroid);

		void apply() override;
		void unapply() override;

	private:

		const cinolib::vec3d m_polyCentroid;
		Dag::Refine* m_operation;

	};

}