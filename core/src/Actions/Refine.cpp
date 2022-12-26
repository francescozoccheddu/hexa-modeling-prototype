#include <HMP/Actions/Refine.hpp>

#include <HMP/Refinement/Utils.hpp>

namespace HMP::Actions
{

	void Refine::apply()
	{
		m_operation->parents().attach(m_element);
		Refinement::Utils::applyRecursive(mesher(), *m_operation);
		mesher().updateMesh();
	}

	void Refine::unapply()
	{
		Refinement::Utils::unapplyRecursive(mesher(), *m_operation);
		mesher().updateMesh();
	}

	Refine::Refine(Dag::Element& _element, I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth)
		: m_element{ _element }, m_operation{ Refinement::Utils::prepare(_forwardFi, _firstVi, _scheme, _depth) }, m_depth{ _depth }
	{
		assert(_depth >= 1 && _depth <= 3);
	}

	const Dag::Element& Refine::element() const
	{
		return m_element;
	}

	const Dag::Refine& Refine::operation() const
	{
		return *m_operation;
	}

	I Refine::depth() const
	{
		return m_depth;
	}

}