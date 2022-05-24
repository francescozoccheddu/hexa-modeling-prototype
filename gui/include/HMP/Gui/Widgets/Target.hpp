#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/gl/canvas_gui_item.h>
#include <cinolib/meshes/drawable_trimesh.h>

namespace HMP::Gui::Widgets
{

	class Target final : public cinolib::SideBarItem
	{

		cinolib::DrawableTrimesh<>* m_mesh;

	public:

		const cinolib::DrawableTrimesh<>& mesh() const;

		void translate(const cinolib::vec3d& _offset);
		void rotate(const cinolib::vec3d& _axis, double _angleRad);
		void scale(double _amount);

		void draw() override;

	};

}