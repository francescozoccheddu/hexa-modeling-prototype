#pragma once

#include <HMP/types.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class MoveVert final : public Commander::Action
	{

	public:

		MoveVert(const Vec& _oldPosition, const Vec& _newPosition);

		void apply() override;
		void unapply() override;

	private:

		const Vec m_oldPosition, m_newPosition;

	};

}