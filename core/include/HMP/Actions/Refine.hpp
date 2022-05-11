#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Refine.hpp>

namespace HMP::Actions
{

	class Refine final : public Commander::Action
	{

	private:

		Dag::Element& m_element;
		Dag::NodeHandle<Dag::Refine> m_operation;

		void apply() override;
		void unapply() override;

	public:

		Refine(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme);

	};

}
