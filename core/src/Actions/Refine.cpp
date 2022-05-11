#include <HMP/Actions/Refine.hpp>

#include <HMP/Actions/Utils.hpp>

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
		Utils::applyRefine(mesher(), *m_operation);
		mesher().updateMesh();
	}

	void Refine::unapply()
	{
		Utils::unapplyRefine(mesher(), *m_operation);
		mesher().updateMesh();
	}

	Refine::Refine(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme)
		: m_element{ _element }, m_operation{ Utils::prepareRefine(_forwardFaceOffset, _upFaceOffset, _scheme) }
	{}

}