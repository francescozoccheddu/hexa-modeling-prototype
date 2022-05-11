#include <HMP/Actions/Rotate.hpp>

#include <HMP/Actions/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>

namespace HMP::Actions
{

	void Rotate::rotate(int _rotation)
	{
		Dag::Element& element{ m_operation.parents().single() };
		Meshing::Utils::removeLeafs(mesher(), m_operation);
		const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
		const Id pid{ mesher().elementToPid(element) };
		const Id forwardFid{ mesh.poly_face_id(pid, m_operation.forwardFaceOffset()) };
		const Id upFid{ mesh.poly_face_id(pid, m_operation.upFaceOffset()) };
		const Id eid{ mesh.face_shared_edge(forwardFid, upFid) };
		const Id newEid{ Meshing::Utils::rotateEid(mesh, forwardFid, eid, _rotation) };
		const Id newUpFid{ Meshing::Utils::adjacentFid(mesh, pid, forwardFid, newEid) };
		m_operation.upFaceOffset() = mesh.poly_face_offset(pid, newUpFid);
		Utils::applyTree(mesher(), m_operation);
		mesher().updateMesh();
	}

	void Rotate::apply()
	{
		rotate(1);
	}

	void Rotate::unapply()
	{
		rotate(-1);
	}

	Rotate::Rotate(Dag::Extrude& _operation)
		: m_operation{ _operation }
	{}

	const Dag::Extrude& Rotate::operation() const
	{
		return m_operation;
	}

}