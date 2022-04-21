#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Dag/Refine.hpp>

namespace HMP::Actions
{

	class Refine3x3 final : public Commander::Action
	{

	public:

		Refine3x3(unsigned int _pid);

		void apply() override;
		void unapply() override;

	private:

		unsigned int m_pid;
		Dag::Refine* m_operation;

	};

}