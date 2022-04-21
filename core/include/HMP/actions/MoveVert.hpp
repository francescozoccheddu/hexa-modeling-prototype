#pragma once

#include <cinolib/geometry/vec_mat.h>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class MoveVert final : public Commander::Action
	{

	public:

		MoveVert(unsigned int _vid, cinolib::vec3d _position);

		void apply() override;
		void unapply() override;

	private:

		const unsigned int m_vid;
		const cinolib::vec3d m_position;
		cinolib::vec3d m_oldPosition;

	};

}