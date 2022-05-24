#include <HMP/Actions/Transform.hpp>

#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <algorithm>
#include <utility>

namespace HMP::Actions
{

	void Transform::apply(const Mat4& _transform)
	{
		Dag::Utils::transform(*root(), _transform);
		Meshing::Utils::addLeafs(mesher(), *root(), true);
		mesher().updateMesh();
	}

	void Transform::apply()
	{
		apply(m_transform);
	}

	void Transform::unapply()
	{
		apply(m_transform.inverse());
	}

	Transform::Transform(const Mat4& _transform)
		: m_transform(_transform)
	{}

	Transform::Transform(Mat4&& _transform)
		: m_transform(std::move(_transform))
	{}

	const Mat4& Transform::transform() const
	{
		return m_transform;
	}

}