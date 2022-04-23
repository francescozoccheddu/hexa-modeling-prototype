#include <HMP/actions/Refine3x3.hpp>

#include <HMP/grid.hpp>
#include <HMP/Refinement/schemes.hpp>
#include <HMP/Utils/Collections.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Refine3x3::Refine3x3(const Vec& _polyCentroid)
		: m_polyCentroid(_polyCentroid)
	{}

	void Refine3x3::apply()
	{
		Grid& grid{ this->grid() };
		Dag::Element& element{ grid.element(grid.mesh.pick_poly(m_polyCentroid)) };
		if (element.children().any([](const Dag::Operation& _child) {return _child.primitive() != Dag::Operation::EPrimitive::Extrude; }))
		{
			throw std::logic_error{ "element has non-extrude child" };
		}
		Dag::Refine& operation{ *new Dag::Refine{} };
		m_operation = &operation;
		element.attachChild(operation);
		const Refinement::Scheme& scheme{ *Refinement::schemes.at(Refinement::EScheme::StandardRefinement) };
		const std::vector<PolyVerts> polys{ scheme.apply(Utils::Collections::toVector(element.vertices())) };
		const std::vector<Dag::Element*> children{ operation.attachChildren(scheme.polyCount()) };
		for (const auto& [child, polyVerts] : Utils::Collections::zip(children, polys))
		{
			grid.addPoly(polyVerts, *child);
		}
		grid.removePoly(element.pid());
		grid.update_mesh();
	}

	void Refine3x3::unapply()
	{
		Grid& grid{ this->grid() };
		for (Dag::Element& child : m_operation->children())
		{
			grid.removePoly(child.pid());
		}
		grid.addPoly(m_operation->parents().single());
		delete m_operation;
	}

}