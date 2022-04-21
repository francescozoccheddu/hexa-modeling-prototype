#pragma once

#include <HMP/Dag/Extrude.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class Extrude final : public Commander::Action
	{

	public:

		Extrude(unsigned int _pid, unsigned int _offset);

		void apply() override;
		void unapply() override;

	private:

		unsigned int m_pid;
		unsigned int m_faceOffset;
		Dag::Extrude* m_operation{};

	};

}