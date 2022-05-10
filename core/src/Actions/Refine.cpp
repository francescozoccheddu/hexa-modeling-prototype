#include <HMP/Actions/Refine.hpp>

#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	Refine::~Refine()
	{
		if (!applied())
		{
			m_operation.children().detachAll(true);
			delete& m_operation;
		}
	}

	void Refine::apply()
	{
		m_operation.parents().attach(m_element);
		Utils::applyRefine(mesher(), m_operation);
		mesher().updateMesh();
	}

	void Refine::unapply()
	{
		Utils::unapplyRefine(mesher(), m_operation);
		mesher().updateMesh();
	}

	Refine::Refine(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme)
		: m_element{ _element }, m_operation{ Utils::prepareRefine(_forwardFaceOffset, _upFaceOffset, _scheme) }
	{}

}