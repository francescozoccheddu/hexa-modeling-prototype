#include <HMP/Actions/Refine.hpp>

#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Meshing/refinementSchemes.hpp>
#include <cpputils/collections/conversions.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Refine::Refine(const Vec& _polyCentroid, const Vec& _faceCentroid, Meshing::ERefinementScheme _scheme)
		: m_polyCentroid(_polyCentroid), m_faceCentroid(_faceCentroid), m_scheme{ _scheme }
	{}

	void Refine::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		Dag::Element& element{ mesher.pidToElement(mesh.pick_poly(m_polyCentroid)) };
		const Id fid{ Meshing::Utils::closestFaceInPoly(mesh, mesher.elementToPid(element), m_faceCentroid) };
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
		const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector(Meshing::Utils::polyVertsFromFace(mesh, mesher.elementToPid(element), fid)))};
		for (const PolyVerts& verts : polys)
		{
			Dag::Element& child{ *new Dag::Element{} };
			child.vertices() = verts;
			operation.children().attach(child);
			mesher.add(child);
		}
		mesher.remove(element);
	}

	void Refine::unapply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		for (Dag::Element& child : m_operation->children())
		{
			mesher.remove(child);
		}
		mesher.add(m_operation->parents().single());
		m_operation->children().detachAll(true);
		delete m_operation;
	}

}