#pragma once

#include <HMP/types.hpp>
#include <vector>
#include <unordered_set>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class MakeConforming : public Commander::Action
	{

	public:

		MakeConforming();

		void apply() override;
		void unapply() override;

	private:

		std::vector<Dag::Refine*> m_operations{};
		std::unordered_set<Dag::Refine*> m_fixedRefines{};

	};

}
