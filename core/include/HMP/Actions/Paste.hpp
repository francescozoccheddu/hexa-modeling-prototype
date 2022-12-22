#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cpputils/collections/FixedVector.hpp>

namespace HMP::Actions
{

	class Paste final: public Commander::Action
	{

	private:

		cpputils::collections::FixedVector<Dag::Element*, 3> m_elements;
		Dag::NodeHandle<Dag::Extrude> m_operation;
		const Dag::Extrude& m_sourceOperation;
		bool m_prepared;

		void apply() override;

		void unapply() override;

	public:

		using Elements = decltype(cpputils::range::ofc(m_elements).dereference().immutable());

		Paste(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Id _vertOffset, bool _clockwise, const Dag::Extrude& _source);

		Elements elements() const;

		const Dag::Extrude& operation() const;

	};

}
