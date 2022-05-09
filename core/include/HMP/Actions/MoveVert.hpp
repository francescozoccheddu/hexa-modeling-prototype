#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class MoveVert final : public Commander::Action
	{

	private:

		Dag::Element& m_element;
		const Id m_vertOffset;
		Vec m_otherPosition;

		void apply() override;
		void unapply() override;

	public:

		MoveVert(Dag::Element& _element, Id _vertOffset, const Vec& _position);

	};

}
