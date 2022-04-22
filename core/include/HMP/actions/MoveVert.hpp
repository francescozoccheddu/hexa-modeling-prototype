#pragma once

#include <cinolib/geometry/vec_mat.h>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class MoveVert final : public Commander::Action
	{

	public:

		MoveVert(const cinolib::vec3d& _oldPosition, const cinolib::vec3d& _newPosition);

		void apply() override;
		void unapply() override;

	private:

		const cinolib::vec3d m_oldPosition, m_newPosition;

	};

}