#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class Delete final : public Commander::Action
	{

	public:
		
		Delete(const Vec& _polyCentroid);

		void apply() override;
		void unapply() override;

	private:

		const Vec m_polyCentroid;
		Dag::Delete* m_operation{};

	};

}
