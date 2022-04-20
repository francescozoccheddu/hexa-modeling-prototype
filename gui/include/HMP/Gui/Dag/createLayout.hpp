#pragma once

#include <HMP/Gui/Dag/Layout.hpp>
#include <HMP/operationstree.hpp>

namespace HMP::Gui::Dag
{

	Layout createLayout(const HMP::OperationsTree& _dag);

}