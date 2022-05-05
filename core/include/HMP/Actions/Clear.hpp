#pragma once

#include <HMP/Dag/Element.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class Clear final : public Commander::Action
	{

	public:
		
		Clear();

		void apply() override;
		void unapply() override;

	private:

		Dag::Element* m_root{};

	};

}
