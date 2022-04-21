#include <HMP/actions/Delete.hpp>

#include <HMP/grid.hpp>

namespace HMP::Actions
{

	Delete::Delete(unsigned int _pid)
		: m_pid(_pid)
	{}

	void Delete::apply()
	{
		Grid& grid{ this->grid() };
		Dag::Element& element{ grid.element(m_pid) };
		if (element.children().any([](const Dag::Operation& _child) {return _child.primitive() != Dag::Operation::EPrimitive::Extrude; }))
		{
			throw std::logic_error{ "element has non-extrude child" };
		}
		Dag::Delete& operation{ *new Dag::Delete{} };
		m_operation = &operation;
		element.attachChild(operation);
		m_vertices = grid.polyVerts(m_pid);
		grid.removePoly(m_pid);
		grid.update_mesh();
	}

	void Delete::unapply()
	{
		grid().addPoly(m_vertices, m_operation->parents().single());
		delete m_operation;
	}

}