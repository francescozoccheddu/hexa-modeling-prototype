#include <HMP/actions/Extrude.hpp>
#include <HMP/grid.hpp>

namespace HMP::Actions
{

	Extrude::Extrude(unsigned int _pid, unsigned int _faceOffset)
		: m_pid{ _pid }, m_faceOffset{ _faceOffset }
	{}

	void Extrude::apply()
	{
		Grid& grid{ this->grid() };
		Dag::Element& element{ grid.element(m_pid) };
		Dag::Extrude& operation{ *new Dag::Extrude{} };
		m_operation = &operation;
		operation.offset() = m_faceOffset;
		Dag::Element& child{ *new Dag::Element{} };
		operation.attachChild(child);
		element.attachChild(operation);
		grid.extrude(m_pid, m_faceOffset, operation);
		grid.update_mesh();
		const std::vector<cinolib::vec3d> verts{ grid.mesh.poly_verts(child.pid()) };
		std::copy(verts.begin(), verts.end(), child.vertices().begin());
	}

	void Extrude::unapply()
	{
		grid().removePoly(m_operation->children().single().pid());
		delete m_operation;
	}

}