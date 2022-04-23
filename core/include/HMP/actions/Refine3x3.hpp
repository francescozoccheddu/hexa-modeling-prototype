#pragma once

#include <HMP/types.hpp>
#include <HMP/Commander.hpp>
#include <HMP/Dag/Refine.hpp>

namespace HMP::Actions
{

	class Refine3x3 final : public Commander::Action
	{

	public:

		Refine3x3(const Vec& _polyCentroid);

		void apply() override;
		void unapply() override;

	private:

		const Vec m_polyCentroid;
		Dag::Refine* m_operation{};

	};

}