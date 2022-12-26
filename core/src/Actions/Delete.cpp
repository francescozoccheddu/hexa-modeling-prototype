#include <HMP/Actions/Delete.hpp>

namespace HMP::Actions
{

	void Delete::apply()
	{
		m_operation->parents().attach(m_element);
		mesher().remove(m_element, false);
		mesher().updateMesh();
	}

	void Delete::unapply()
	{
		m_operation->parents().detachAll(false);
		mesher().add_TOPM(m_element);
		mesher().updateMesh();
	}

	Delete::Delete(Dag::Element& _element)
		: m_element{ _element }, m_operation{ *new Dag::Delete{} }
	{}

	const Dag::Element& Delete::element() const
	{
		return m_element;
	}

	const Dag::Delete& Delete::operation() const
	{
		return *m_operation;
	}

}