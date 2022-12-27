#include <HMP/Actions/Refine.hpp>

#include <HMP/Refinement/Utils.hpp>
#include <cassert>

namespace HMP::Actions
{

	void Refine::apply()
	{
		assert(mesher().shown(m_element));
		m_oldState = mesher().state();
		m_operation->parents.attach(m_element);
		Refinement::Utils::applyRecursive(mesher(), *m_operation);
		mesher().updateMesh();
	}

	void Refine::unapply()
	{
		m_operation->parents.detachAll(false);
		mesher().show(m_element, true);
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	Refine::Refine(Dag::Element& _element, I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth)
		: m_element{ _element }, m_operation{ Refinement::Utils::prepare(_forwardFi, _firstVi, _scheme, _depth) }, m_depth{ _depth }
	{
		assert(_depth >= 1 && _depth <= 3);
		assert(_forwardFi < 6);
		assert(_firstVi < 8);
		assert(Refinement::schemes.contains(_scheme));
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