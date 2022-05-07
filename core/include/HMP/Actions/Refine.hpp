#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Refine.hpp>

namespace HMP::Actions
{

	class Refine final : public Commander::Action
	{

	private:

		Dag::Element& m_element;
		Dag::Refine& m_operation;

		~Refine() override;

		void apply() override;
		void unapply() override;

	public:

		static Dag::Refine& prepareRefine(Id _faceOffset, Meshing::ERefinementScheme _scheme);
		static void applyRefine(Meshing::Mesher& _mesher, Dag::Element& _element, Dag::Refine& _refine);
		static void unapplyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine);

		Refine(Dag::Element& _element, Id _faceOffset, Meshing::ERefinementScheme _scheme);

	};

}
