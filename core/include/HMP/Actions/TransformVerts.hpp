#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>
#include <HMP/Dag/Element.hpp>
#include <vector>
#include <utility>

namespace HMP::Actions
{

	class TransformVerts final : public Commander::Action
	{

	public:

		class Vert final
		{

		private:

			Dag::Element* m_element;
			Id m_vertOffset;

		public:

			Vert(Dag::Element& _element, Id _vertOffset);

			Dag::Element& element();

			const Dag::Element& element() const;

			Id vertOffset() const;

		};

	private:

		const Mat4 m_transform;
		std::vector<Vert> m_verts;

		void apply(const Mat4& _transform);
		void apply() override;
		void unapply() override;

	public:

		TransformVerts(const Mat4& _transform, const std::vector<Vert> _verts);
		TransformVerts(Mat4&& _transform, const std::vector<Vert> _verts);
		TransformVerts(const Mat4& _transform, std::vector<Vert>&& _verts);
		TransformVerts(Mat4&& _transform, std::vector<Vert>&& _verts);

		const Mat4& transform() const;

		const std::vector<Vert>& verts() const;

	};

}
