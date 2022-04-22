#include <HMP/actions/Extrude.hpp>

#include <HMP/grid.hpp>
#include <array>
#include <stdexcept>
#include <algorithm>

namespace HMP::Actions
{

	Extrude::Extrude(const cinolib::vec3d& _polyCentroid, const cinolib::vec3d& _faceCentroid)
		: m_polyCentroid(_polyCentroid), m_faceCentroid(_faceCentroid)
	{}

	void Extrude::apply()
	{
		Grid& grid{ this->grid() };
		Dag::Element& element{ grid.element(grid.mesh.pick_poly(m_polyCentroid)) };
		const unsigned int fid{ grid.closestPolyFid(element.pid(), m_faceCentroid) };
		const unsigned int faceOffset{ grid.mesh.poly_face_offset(element.pid(), fid) };
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
		operation.attachChild(child);
		element.attachChild(operation);
		{
			double avgFaceEdgeLength{};
			{
				const std::vector<unsigned int> faceEids{ grid.mesh.adj_f2e(fid) };
				for (const unsigned int eid : faceEids)
				{
					avgFaceEdgeLength += grid.mesh.edge_length(eid);
				}
				avgFaceEdgeLength /= faceEids.size();
			}

			std::array<cinolib::vec3d, 8> verts;
			const std::vector<cinolib::vec3d> faceVerts = grid.mesh.face_verts(fid);
			std::copy(faceVerts.begin(), faceVerts.end(), verts.begin());
			const cinolib::vec3d faceNormal = grid.mesh.face_data(fid).normal;
			int i{ 4 };
			for (const cinolib::vec3d& faceVert : faceVerts)
			{
				verts[i++] = faceVert + faceNormal * avgFaceEdgeLength;
			}
			grid.addPoly(verts, child);
			grid.update_mesh();
		}
	}

	void Extrude::unapply()
	{
		grid().removePoly(m_operation->children().single().pid());
		delete m_operation;
	}

}