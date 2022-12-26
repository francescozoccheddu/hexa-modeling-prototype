#pragma once

#include <HMP/Dag/Node.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Actions/Root.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Paste.hpp>
#include <HMP/Actions/Project.hpp>
#include <HMP/Actions/Refine.hpp>
#include <HMP/Actions/Transform.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <cpputils/collections/Namer.hpp>
#include <string>

namespace HMP::Gui::Utils::HrDescriptions
{

	using DagNamer = cpputils::collections::Namer<const HMP::Dag::Node*>;

	std::string name(const HMP::Dag::Node& _node, DagNamer& _dagNamer);
	std::string describe(Refinement::EScheme _scheme);
	std::string describe(const Vec& _vec);
	std::string describe(const Mat4& _mat);
	std::string describe(const std::vector<I>& _is);
	std::string describe(const std::vector<Id>& _ids);
	std::string describe(const std::vector<bool>& _flags);
	std::string describe(const HMP::Dag::Delete& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Extrude& _operation, const cpputils::collections::FixedVector<const HMP::Dag::Element*, 3>& _elements, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Refine& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Delete& _operation, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Extrude& _operation, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Refine& _operation, DagNamer& _dagNamer);

	std::string describe(const Actions::Delete& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Extrude& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Root& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::MakeConforming& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Paste& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Project& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Refine& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Transform& _action, DagNamer& _dagNamer);
	std::string describe(const Commander::Action& _action, DagNamer& _dagNamer);

}