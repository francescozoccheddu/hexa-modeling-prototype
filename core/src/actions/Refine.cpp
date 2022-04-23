#include <HMP/actions/Refine.hpp>

#include <HMP/grid.hpp>
#include <HMP/Refinement/schemes.hpp>
#include <HMP/Utils/Collections.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Refine::Refine(const Vec& _polyCentroid, const Vec& _faceCentroid, Refinement::EScheme _scheme)
		: m_polyCentroid(_polyCentroid), m_faceCentroid(_faceCentroid), m_scheme{ _scheme }
	{}

	void Refine::apply()
	{
		Grid& grid{ this->grid() };
		Dag::Element& element{ grid.element(grid.mesh.pick_poly(m_polyCentroid)) };
		const Id fid{ grid.closestPolyFid(element.pid(), m_faceCentroid) };
		const Id faceOffset{ grid.mesh.poly_face_offset(element.pid(), fid) };
		if (element.children().any([](const Dag::Operation& _child) {return _child.primitive() != Dag::Operation::EPrimitive::Extrude; }))
		{
			throw std::logic_error{ "element has non-extrude child" };
		}
		Dag::Refine& operation{ *new Dag::Refine{} };
		operation.scheme() = m_scheme;
		m_operation = &operation;
		element.attachChild(operation);
		const Refinement::Scheme& scheme{ *Refinement::schemes.at(m_scheme) };
		const std::vector<PolyVerts> polys{ scheme.apply(Utils::Collections::toVector(grid.polyVerts(element.pid(), faceOffset))) };
		const std::vector<Dag::Element*> children{ operation.attachChildren(scheme.polyCount()) };
		for (const auto& [child, polyVerts] : Utils::Collections::zip(children, polys))
		{
			grid.addPoly(polyVerts, *child);
		}
		grid.removePoly(element.pid());
		grid.update_mesh();
	}

	void Refine::unapply()
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