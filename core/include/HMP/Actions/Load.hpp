#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>

namespace HMP::Actions
{

	class Load final : public Commander::Action
	{

	private:

		Dag::NodeHandle<Dag::Element> m_otherRoot;

		void apply() override;
		void unapply() override;

	public:
		
		Load(Dag::Element& _root);

	};

}
