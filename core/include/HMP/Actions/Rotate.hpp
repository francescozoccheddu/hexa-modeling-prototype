#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/Extrude.hpp>

namespace HMP::Actions
{

	class Rotate final : public Commander::Action
	{

	private:

		Dag::Extrude& m_operation;

		void rotate(int _rotation);

		void apply() override;
		void unapply() override;

	public:

		Rotate(Dag::Extrude& _operation);

		const Dag::Extrude& operation() const;

	};

}
