#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <cpputils/collections/DereferenceIterable.hpp>
#include <array>

namespace HMP::Actions
{

	class Extrude final: public Commander::Action
	{

	public:

		using Elements = cpputils::collections::DereferenceIterable<const std::vector<Dag::Element*>, const Dag::Element&>;

	private:

		std::vector<Dag::Element*> m_elements;
		Dag::NodeHandle<Dag::Extrude> m_operation;

		void apply() override;
		void unapply() override;

		Extrude(const std::vector<Dag::Element*>& _elements, const std::array<Id, 3>& _faceOffsets, Dag::Extrude::ESource _source);

	public:

		Extrude(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset);
		Extrude(Dag::Element& _element0, Id _faceOffset0, Dag::Element& _element1, Id _faceOffset1);
		Extrude(Dag::Element& _element0, Id _faceOffset0, Dag::Element& _element1, Id _faceOffset1, Dag::Element& _element2, Id _faceOffset2);

		Elements elements() const;

		const Dag::Extrude& operation() const;

	};

}
