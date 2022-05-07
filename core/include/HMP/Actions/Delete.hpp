#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Delete.hpp>

namespace HMP::Actions
{

	class Delete final : public Commander::Action
	{

	private:

		Dag::Element& m_element;
		Dag::Delete& m_operation;

		~Delete() override;

		void apply() override;
		void unapply() override;

	public:
		
		Delete(Dag::Element& _element);

	};

}
