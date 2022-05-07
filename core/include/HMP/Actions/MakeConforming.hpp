#pragma once

#include <HMP/Meshing/types.hpp>
#include <vector>
#include <unordered_set>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class MakeConforming final : public Commander::Action
	{

	private:

		std::vector<Dag::Refine*> m_operations{};

		~MakeConforming() override;

		void apply() override;
		void unapply() override;

	public:

		MakeConforming();

	};

}
