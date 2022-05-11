#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>

namespace HMP::Actions
{

	class Paste final : public Commander::Action
	{

	private:

		Dag::Element& m_element;
		Dag::NodeHandle<Dag::Extrude> m_operation;
		bool m_prepared;

		void apply() override;
		void unapply() override;

	public:
		
		Paste(Dag::Element& _target, Id _targetForwardFaceOffset, Id _targetUpFaceOffset, Dag::Extrude& _source);

	};

}
