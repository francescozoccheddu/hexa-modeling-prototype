#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/Element.hpp>

namespace HMP::Actions
{

	class Clear final : public Commander::Action
	{

	private:

		Dag::Element* m_otherRoot;

		~Clear() override;

		void apply() override;
		void unapply() override;

	public:
		
		Clear();

	};

}
