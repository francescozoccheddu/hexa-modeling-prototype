#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>

namespace HMP::Actions
{

	class Paste final : public Commander::Action
	{

	private:

		Dag::Extrude& m_operation;

		~Paste() override;

		void apply() override;
		void unapply() override;

	public:
		
		Paste(Dag::Element& _target, Id _targetForwardFaceOffset, Id _targetUpFaceOffset, Dag::Extrude& _source, Id _sourceUpFaceOffset);

	};

}
