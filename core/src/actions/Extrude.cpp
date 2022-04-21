#include <HMP/actions/Extrude.hpp>

#include <HMP/grid.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Extrude::Extrude(unsigned int _pid, unsigned int _faceOffset)
		: m_pid{ _pid }, m_faceOffset{ _faceOffset }
	{}

	void Extrude::apply()
	{
		Grid& grid{ this->grid() };
		Dag::Element& element{ grid.element(m_pid) };
		for (const Dag::Operation& child : element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
			if (static_cast<const Dag::Extrude&>(child).faceOffset() == m_faceOffset)
			{
				throw std::logic_error{ "element already has equivalent child" };
			}
		}
		Dag::Extrude& operation{ *new Dag::Extrude{} };
		m_operation = &operation;
		operation.faceOffset() = m_faceOffset;
		Dag::Element& child{ *new Dag::Element{} };
		operation.attachChild(child);
		element.attachChild(operation);
		{
			const unsigned int fid = grid.mesh.poly_face_id(m_pid, m_faceOffset);

			double avgFaceEdgeLength{};
			{
				const std::vector<unsigned int> faceEids{ grid.mesh.adj_f2e(fid) };
				for (const unsigned int eid : faceEids)
				{
					avgFaceEdgeLength += grid.mesh.edge_length(eid);
				}
				avgFaceEdgeLength /= faceEids.size();
			}

			const std::vector<cinolib::vec3d> faceVerts = grid.mesh.face_verts(fid);
			std::copy(faceVerts.begin(), faceVerts.end(), child.vertices().begin());
			const cinolib::vec3d faceNormal = grid.mesh.face_data(fid).normal;
			std::array<unsigned int, 8> vids;
			{
				const std::vector<unsigned int> faceVids{ grid.mesh.face_verts_id(fid) };
				std::copy(faceVids.begin(), faceVids.end(), vids.begin());
			}
			int i{ 4 };
			for (const cinolib::vec3d& faceVert : faceVerts)
			{
				const cinolib::vec3d vert{ faceVert + faceNormal * avgFaceEdgeLength };
				element.vertices()[i] = vert;
				vids[i] = grid.addOrGetVert(vert);
				i++;
			}

			grid.addPoly(vids, child);
			grid.update_mesh();
		}
	}

	void Extrude::unapply()
	{
		grid().removePoly(m_operation->children().single().pid());
		delete m_operation;
	}

}