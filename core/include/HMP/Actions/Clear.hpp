#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/NodeHandle.hpp>

namespace HMP::Actions
{

	class Clear final : public Commander::Action
	{

	private:

		Dag::NodeHandle<Dag::Element> m_otherRoot;

		void apply() override;
		void unapply() override;

	public:
		
		Clear();

	};

}
