#include <HMP/Actions/Delete.hpp>

#include <HMP/Meshing/Mesher.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Delete::Delete(const Vec& _polyCentroid)
		: m_polyCentroid(_polyCentroid)
	{}

	void Delete::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		Dag::Element& element{ mesher.pidToElement(mesh.pick_poly(m_polyCentroid)) };
		if (element.isRoot())
		{
			throw std::logic_error{ "element is root" };
		}
		for (const Dag::Operation& child : element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
		}
		Dag::Delete& operation{ *new Dag::Delete{} };
		m_operation = &operation;
		element.children().attach(operation);
		mesher.remove(element);
	}

	void Delete::unapply()
	{
		mesher().add(m_operation->parents().single());
		m_operation->children().detachAll(true);
		delete m_operation;
	}

}