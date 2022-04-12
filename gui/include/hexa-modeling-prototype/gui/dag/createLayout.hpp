#pragma once

#include <hexa-modeling-prototype/gui/dag/Layout.hpp>
#include <hexa-modeling-prototype/operationstree.hpp>

namespace HMP::Gui::Dag
{

	Layout createLayout(const HMP::OperationsTree& _dag);

}