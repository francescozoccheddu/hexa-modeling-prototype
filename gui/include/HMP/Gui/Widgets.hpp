#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Commander.hpp>
#include <HMP/Gui/HrDescriptions.hpp>
#include <cinolib/gl/FreeCamera.hpp>

namespace HMP::Gui::Widgets
{

	void drawCommanderControls(Commander& _commander, HrDescriptions::DagNamer& _dagNamer);

	void drawAxes(const cinolib::FreeCamera<Real>& _camera);

}