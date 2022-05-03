#pragma once

#include <HMP/types.hpp>
#include <HMP/Commander.hpp>
#include <HMP/Dag/Refine.hpp>

namespace HMP::Actions
{

	class Refine final : public Commander::Action
	{

	public:

		Refine(const Vec& _polyCentroid, const Vec& _faceCentroid, Meshing::ERefinementScheme _scheme);

		void apply() override;
		void unapply() override;

	private:

		const Vec m_polyCentroid, m_faceCentroid;
		const Meshing::ERefinementScheme m_scheme;
		Dag::Refine* m_operation{};

	};

}
