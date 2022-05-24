#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>

namespace HMP::Actions
{

	class Transform final : public Commander::Action
	{

	private:

		const Mat4 m_transform;

		void apply(const Mat4& _transform);
		void apply() override;
		void unapply() override;

	public:
		
		Transform(const Mat4& _transform);
		Transform(Mat4&& _transform);

		const Mat4& transform() const;

	};

}
