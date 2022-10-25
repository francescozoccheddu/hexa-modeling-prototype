#include <HMP/Actions/TransformVerts.hpp>

#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <algorithm>
#include <utility>

namespace HMP::Actions
{

	void TransformVerts::apply(const Mat4& _transform)
	{
		Dag::Utils::transform(*root(), _transform);
		Meshing::Utils::addLeafs(mesher(), *root(), true);
		mesher().updateMesh();
	}

	void TransformVerts::apply()
	{
		apply(m_transform);
	}

	void TransformVerts::unapply()
	{
		apply(m_transform.inverse());
	}

	const Mat4& TransformVerts::transform() const
	{
		return m_transform;
	}

	const std::vector<TransformVerts::Vert>& TransformVerts::verts() const
	{
		return m_verts;
	}

}