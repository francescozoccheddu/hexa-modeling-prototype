#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/Extrude.hpp>

namespace HMP::Actions
{

	class Extrude final : public Commander::Action
	{

	private:

		Dag::Element& m_element;
		Dag::Extrude& m_operation;

		~Extrude() override;

		void apply() override;
		void unapply() override;

	public:

		Extrude(Dag::Element& _element, Id _faceOffset);

	};

}
