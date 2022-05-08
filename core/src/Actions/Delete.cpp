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
		if (m_element.isRoot())
		{
			throw std::logic_error{ "element is root" };
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