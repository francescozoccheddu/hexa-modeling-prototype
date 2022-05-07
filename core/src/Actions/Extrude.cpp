#include <HMP/Actions/Extrude.hpp>

#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <array>
#include <stdexcept>
#include <algorithm>

namespace HMP::Actions
{

	Extrude::~Extrude()
	{
		if (!applied())
		{
			m_operation.children().detachAll(true);
			delete& m_operation;
		}
	}

	void Extrude::apply()
	{
		for (const Dag::Operation& child : m_element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
			if (static_cast<const Dag::Extrude&>(child).faceOffset() == m_operation.faceOffset())
			{
				throw std::logic_error{ "element already has equivalent child" };
			}
		}
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		const Id fid{ mesh.poly_face_id(mesher.elementToPid(m_element), m_operation.faceOffset()) };
		Real avgFaceEdgeLength{};
		{
			const std::vector<Id> faceEids{ mesh.adj_f2e(fid) };
			for (const Id eid : faceEids)
			{
				avgFaceEdgeLength += mesh.edge_length(eid);
			}
			avgFaceEdgeLength /= faceEids.size();
		}
		PolyVerts& verts{ m_operation.children().single().vertices() };
		const std::vector<Vec> faceVerts = mesh.face_verts(fid);
		std::copy(faceVerts.begin(), faceVerts.end(), verts.begin());
		const Vec faceNormal = mesh.face_data(fid).normal;
		int i{ 4 };
		for (const Vec& faceVert : faceVerts)
		{
			verts[i++] = faceVert + faceNormal * avgFaceEdgeLength;
		}
		mesher.add(m_operation.children().single());
		m_element.children().attach(m_operation);
	}

	void Extrude::unapply()
	{
		m_operation.parents().detachAll(false);
		mesher().remove(m_operation.children().single());
	}

	Extrude::Extrude(Dag::Element& _element, Id _faceOffset)
		: m_element{ _element }, m_operation{ *new Dag::Extrude{} }
	{
		m_operation.faceOffset() = _faceOffset;
		m_operation.children().attach(*new Dag::Element{});
	}

}