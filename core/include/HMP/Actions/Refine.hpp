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
		std::size_t m_depth;

		void apply() override;
		void unapply() override;

	public:

		Refine(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme, std::size_t _depth = 1);

		const Dag::Element& element() const;
		const Dag::Refine& operation() const;
		std::size_t depth() const;

	};

}
