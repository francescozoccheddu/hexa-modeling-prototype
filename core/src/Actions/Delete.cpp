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
		Grid::Mesh& mesh{ grid.mesh() };
		Dag::Element& element{ grid.element(mesh.pick_poly(m_polyCentroid)) };
		if (element.isRoot())
		{
			throw std::logic_error{ "element is root" };
		}
		for (Dag::Operation& child : element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
		}
		Dag::Delete& operation{ *new Dag::Delete{} };
		m_operation = &operation;
		element.children().attach(operation);
		grid.removePoly(element.pid());
		grid.mesh().updateGL();
	}

	void Delete::unapply()
	{
		grid().addPoly(m_operation->parents().single());
		grid().mesh().updateGL();
		delete m_operation;
	}

}