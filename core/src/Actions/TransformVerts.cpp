#include <HMP/Actions/TransformVerts.hpp>

#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <algorithm>
#include <utility>
#include <stdexcept>

namespace HMP::Actions
{

	TransformVerts::Vert::Vert(Dag::Element& _element, Id _vertOffset) : m_element{ &_element }, m_vertOffset{ _vertOffset }
	{
		if (m_vertOffset >= 8)
		{
			throw std::domain_error{ "bad vertex offset" };
		}
	}

	Dag::Element& TransformVerts::Vert::element()
	{
		return *m_element;
	}

	const Dag::Element& TransformVerts::Vert::element() const
	{
		return *m_element;
	}

	Id TransformVerts::Vert::vertOffset() const
	{
		return m_vertOffset;
	}

	void TransformVerts::apply(const Mat4& _transform)
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		for (Vert& vert : m_verts)
		{
			const Id pid{ mesher.elementToPid(vert.element()) };
			const Id vid{ mesh.poly_vert_id(pid, vert.vertOffset()) };
			mesher.moveVert(vid, _transform * mesh.vert(vid));
		}
		mesher.updateMesh();
	}

	void TransformVerts::apply()
	{
		apply(m_transform);
	}

	void TransformVerts::unapply()
	{
		apply(m_transform.inverse());
	}

	TransformVerts::TransformVerts(const Mat4& _transform, const std::vector<Vert> _verts)
		: m_transform{ _transform }, m_verts{ _verts }
	{}

	TransformVerts::TransformVerts(Mat4&& _transform, const std::vector<Vert> _verts)
		: m_transform{ std::move(_transform) }, m_verts{ _verts }
	{}

	TransformVerts::TransformVerts(const Mat4& _transform, std::vector<Vert>&& _verts)
		: m_transform{ _transform }, m_verts{ std::move(_verts) }
	{}

	TransformVerts::TransformVerts(Mat4&& _transform, std::vector<Vert>&& _verts)
		: m_transform{ std::move(_transform) }, m_verts{ std::move(_verts) }
	{}

	const Mat4& TransformVerts::transform() const
	{
		return m_transform;
	}

	const std::vector<TransformVerts::Vert>& TransformVerts::verts() const
	{
		return m_verts;
	}

}