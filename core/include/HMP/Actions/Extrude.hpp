#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cpputils/range/of.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <type_traits>
#include <array>

namespace HMP::Actions
{

	class Extrude final: public Commander::Action
	{

	private:

		cpputils::collections::FixedVector<Dag::Element*, 3> m_elements;
		Dag::NodeHandle<Dag::Extrude> m_operation;

		void apply() override;
		void unapply() override;


	public:

		using Elements = decltype(cpputils::range::ofc(m_elements).dereference().immutable());

		Extrude(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Id _vertOffset);

		Elements elements() const;

		const Dag::Extrude& operation() const;

	};

}
