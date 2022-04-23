#pragma once

#include <HMP/types.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class Extrude final : public Commander::Action
	{

	public:

		Extrude(const Vec& _polyCentroid, const Vec& _faceCentroid);

		void apply() override;
		void unapply() override;

	private:

		const Vec m_polyCentroid, m_faceCentroid;
		Dag::Extrude* m_operation{};

	};

}