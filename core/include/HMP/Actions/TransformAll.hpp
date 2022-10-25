#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class TransformAll final : public Commander::Action
	{

	private:

		const Mat4 m_transform;

		void apply(const Mat4& _transform);
		void apply() override;
		void unapply() override;

	public:
		
		TransformAll(const Mat4& _transform);
		TransformAll(Mat4&& _transform);

		const Mat4& transform() const;

	};

}
