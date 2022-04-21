#pragma once

#include <cinolib/geometry/vec_mat.h>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class MoveVert : public Commander::Action
	{

	public:

		MoveVert(unsigned int _vid, cinolib::vec3d _position);

		void apply() override;
		void unapply() override;

	private:

		unsigned int m_vid;
		cinolib::vec3d m_position;
		cinolib::vec3d m_oldPosition;

	};

}