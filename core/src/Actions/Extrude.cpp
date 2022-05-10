#include <HMP/Actions/Extrude.hpp>

#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	Extrude::~Extrude()
	{
		if (!applied())
		{
			m_operation.children().detachAll(true);
			delete& m_operation;
		}
	}

	void Extrude::apply()
	{
		Utils::applyExtrude(mesher(), m_element, m_operation);
		mesher().updateMesh();
	}

	void Extrude::unapply()
	{
		Utils::unapplyExtrude(mesher(), m_operation);
		mesher().updateMesh();
	}

	Extrude::Extrude(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset)
		: m_element{ _element }, m_operation{ Utils::prepareExtrude(_forwardFaceOffset, _upFaceOffset) }
	{}

}