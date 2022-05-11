#include <HMP/Actions/Delete.hpp>

#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	void Delete::apply()
	{
		if (mesher().mesh().num_polys() == 1)
		{
			throw std::logic_error{ "cannot delete the only active element" };
		}
		for (const Dag::Operation& child : m_element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
		}
		m_operation->parents().attach(m_element);
		Utils::applyDelete(mesher(), *m_operation);
		mesher().updateMesh();
	}

	void Delete::unapply()
	{
		Utils::unapplyDelete(mesher(), *m_operation);
		mesher().updateMesh();
	}

	Delete::Delete(Dag::Element& _element)
		: m_element{ _element }, m_operation{ Utils::prepareDelete() }
	{}

}