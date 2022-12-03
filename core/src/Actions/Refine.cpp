#include <HMP/Actions/Refine.hpp>

#include <HMP/Actions/Utils.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	void Refine::apply()
	{
		for (const Dag::Operation& child : m_element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
		}
		m_operation->parents().attach(m_element);
		Utils::applyRefineRecursive(mesher(), *m_operation);
		mesher().updateMesh();
	}

	void Refine::unapply()
	{
		Utils::unapplyRefineRecursive(mesher(), *m_operation);
		mesher().updateMesh();
	}

	Refine::Refine(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme, I _depth)
		: m_element{ _element }, m_operation{ Utils::prepareRefine(_forwardFaceOffset, _upFaceOffset, _scheme, _depth) }, m_depth{ _depth }
	{
		if (_depth < 1 || _depth > 3)
		{
			throw std::logic_error{ "depth must be in range [1, 3]" };
		}
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