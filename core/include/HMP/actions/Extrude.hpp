#pragma once

#include <HMP/Dag/Extrude.hpp>
#include <cinolib/geometry/vec_mat.h>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class Extrude final : public Commander::Action
	{

	public:

		Extrude(const cinolib::vec3d& _polyCentroid, const cinolib::vec3d& _faceCentroid);

		void apply() override;
		void unapply() override;

	private:

		const cinolib::vec3d m_polyCentroid, m_faceCentroid;
		Dag::Extrude* m_operation{};

	};

}