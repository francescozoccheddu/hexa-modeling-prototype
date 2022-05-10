#include <HMP/Actions/Delete.hpp>

#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	Delete::~Delete()
	{
		if (!applied())
		{
			m_operation.children().detachAll(true);
			delete& m_operation;
		}
	}

	void Delete::apply()
	{
		Utils::applyDelete(mesher(), m_element, m_operation);
		mesher().updateMesh();
	}

	void Delete::unapply()
	{
		Utils::unapplyDelete(mesher(), m_operation);
		mesher().updateMesh();
	}

	Delete::Delete(Dag::Element& _element)
		: m_element{ _element }, m_operation{ Utils::prepareDelete() }
	{}

}