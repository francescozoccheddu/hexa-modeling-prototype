#include <HMP/Actions/Delete.hpp>

#include <stdexcept>

namespace HMP::Actions
{

	Delete::~Delete()
	{
		if (!applied())
		{
			m_operation.children().detachAll(true);
			delete &m_operation;
		}
	}

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
		m_element.children().attach(m_operation);
		mesher().remove(m_element);
		mesher().updateMesh();
	}

	void Delete::unapply()
	{
		m_operation.parents().detachAll(false);
		mesher().add(m_element);
		mesher().updateMesh();
	}

	Delete::Delete(Dag::Element& _element)
		: m_element{ _element }, m_operation{ *new Dag::Delete{} }
	{}

}