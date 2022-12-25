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
		I m_depth;

		void apply() override;
		void unapply() override;

	public:

		Refine(Dag::Element& _element, I _forwardFi, I _firstVi, Meshing::ERefinementScheme _scheme, I _depth = 1);

		const Dag::Element& element() const;
		const Dag::Refine& operation() const;
		I depth() const;

	};

}
