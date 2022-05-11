#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Extrude.hpp>

namespace HMP::Actions
{

	class Extrude final : public Commander::Action
	{

	private:

		Dag::Element& m_element;
		Dag::NodeHandle<Dag::Extrude> m_operation;

		void apply() override;
		void unapply() override;

	public:

		Extrude(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset);

	};

}
