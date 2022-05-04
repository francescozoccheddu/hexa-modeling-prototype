#include <HMP/Actions/Refine.hpp>

#include <HMP/grid.hpp>
#include <HMP/Meshing/refinementSchemes.hpp>
#include <HMP/Utils/Collections.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Refine::Refine(const Vec& _polyCentroid, const Vec& _faceCentroid, Meshing::ERefinementScheme _scheme)
		: m_polyCentroid(_polyCentroid), m_faceCentroid(_faceCentroid), m_scheme{ _scheme }
	{}

	void Refine::apply()
	{
		Grid& grid{ this->grid() };
		Grid::Mesh& mesh{ grid.mesh() };
		Dag::Element& element{ grid.element(mesh.pick_poly(m_polyCentroid)) };
		const Id fid{ grid.closestPolyFid(element.pid(), m_faceCentroid) };
		const Id faceOffset{ mesh.poly_face_offset(element.pid(), fid) };
		for (const Dag::Operation& child : element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
		}
		Dag::Refine& operation{ *new Dag::Refine{} };
		operation.scheme() = m_scheme;
		operation.needsTopologyFix() = true;
		m_operation = &operation;
		element.children().attach(operation);
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(m_scheme) };
		const std::vector<PolyVerts> polys{ refinement.apply(Utils::Collections::toVector(grid.polyVertsFromFace(element.pid(), faceOffset))) };
		for (std::size_t i{ 0 }; i < refinement.polyCount(); i++)
		{
			Dag::Element& child{ *new Dag::Element{} };
			operation.children().attach(child);
			grid.addPoly(polys[i], child);
		}
		grid.removePoly(element.pid());
		mesh.updateGL();
	}

	void Refine::unapply()
	{
		Grid& grid{ this->grid() };
		for (const Dag::Element& child : m_operation->children())
		{
			grid.removePoly(child.pid());
		}
		grid.addPoly(m_operation->parents().single());
		delete m_operation;
	}

}