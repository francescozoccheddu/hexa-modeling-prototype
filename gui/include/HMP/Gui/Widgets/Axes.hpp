#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/FreeCamera.hpp>
#include <cinolib/gl/canvas_gui_item.h>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP::Gui::Widgets
{

	class Axes final : public cinolib::CanvasGuiItem, public cpputils::mixins::ReferenceClass
	{

	private:

		const cinolib::FreeCamera<Real>& m_camera;

	public:

		explicit Axes(const cinolib::FreeCamera<Real>& _camera);

		const cinolib::FreeCamera<Real>& camera() const;

		void draw() override;

	};

}