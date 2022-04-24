#include <HMP/Actions/Delete.hpp>

#include <HMP/grid.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Delete::Delete(const Vec& _polyCentroid)
		: m_polyCentroid(_polyCentroid)
	{}

	void Delete::apply()
	{
		Grid& grid{ this->grid() };
		Dag::Element& element{ grid.element(grid.mesh.pick_poly(m_polyCentroid)) };
		if (element.isRoot())
		{
			throw std::logic_error{ "element is root" };
		}
		if (element.children().any([](const Dag::Operation& _child) {return _child.primitive() != Dag::Operation::EPrimitive::Extrude; }))
		{
			throw std::logic_error{ "element has non-extrude child" };
		}
		Dag::Delete& operation{ *new Dag::Delete{} };
		m_operation = &operation;
		element.attachChild(operation);
		grid.removePoly(element.pid());
		grid.update_mesh();
	}

	void Delete::unapply()
	{
		grid().addPoly(m_operation->parents().single());
		grid().update_mesh();
		delete m_operation;
	}

}