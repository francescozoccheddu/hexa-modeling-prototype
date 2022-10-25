#include <HMP/Actions/TransformAll.hpp>

#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <algorithm>
#include <utility>

namespace HMP::Actions
{

	void TransformAll::apply(const Mat4& _transform)
	{
		Dag::Utils::transform(*root(), _transform);
		Meshing::Utils::addLeafs(mesher(), *root(), true);
		mesher().updateMesh();
	}

	void TransformAll::apply()
	{
		apply(m_transform);
	}

	void TransformAll::unapply()
	{
		apply(m_transform.inverse());
	}

	TransformAll::TransformAll(const Mat4& _transform)
		: m_transform(_transform)
	{}

	TransformAll::TransformAll(Mat4&& _transform)
		: m_transform(std::move(_transform))
	{}

	const Mat4& TransformAll::transform() const
	{
		return m_transform;
	}

}