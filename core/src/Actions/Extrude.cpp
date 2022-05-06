#include <HMP/Actions/Extrude.hpp>

#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <array>
#include <stdexcept>
#include <algorithm>

namespace HMP::Actions
{

	Extrude::Extrude(const Vec& _polyCentroid, const Vec& _faceCentroid)
		: m_polyCentroid(_polyCentroid), m_faceCentroid(_faceCentroid)
	{}

	void Extrude::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		Dag::Element& element{ mesher.pidToElement(mesh.pick_poly(m_polyCentroid)) };
		const Id fid{ Meshing::Utils::closestFaceInPoly(mesh, mesher.elementToPid(element), m_faceCentroid) };
		const Id faceOffset{ mesh.poly_face_offset(mesher.elementToPid(element), fid) };
		for (const Dag::Operation& child : element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
			if (static_cast<const Dag::Extrude&>(child).faceOffset() == faceOffset)
			{
				throw std::logic_error{ "element already has equivalent child" };
			}
		}
		Dag::Extrude& operation{ *new Dag::Extrude{} };
		m_operation = &operation;
		operation.faceOffset() = faceOffset;
		Dag::Element& child{ *new Dag::Element{} };
		operation.children().attach(child);
		element.children().attach(operation);
		{
			Real avgFaceEdgeLength{};
			{
				const std::vector<Id> faceEids{ mesh.adj_f2e(fid) };
				for (const Id eid : faceEids)
				{
					avgFaceEdgeLength += mesh.edge_length(eid);
				}
				avgFaceEdgeLength /= faceEids.size();
			}

			PolyVerts& verts{ child.vertices() };
			const std::vector<Vec> faceVerts = mesh.face_verts(fid);
			std::copy(faceVerts.begin(), faceVerts.end(), verts.begin());
			const Vec faceNormal = mesh.face_data(fid).normal;
			int i{ 4 };
			for (const Vec& faceVert : faceVerts)
			{
				verts[i++] = faceVert + faceNormal * avgFaceEdgeLength;
			}
			mesher.add(child);
		}
	}

	void Extrude::unapply()
	{
		mesher().remove(m_operation->children().single());
		m_operation->children().detachAll(true);
		delete m_operation;
	}

}