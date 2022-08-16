#pragma once

#include <HMP/Dag/Node.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Actions/MoveVert.hpp>
#include <HMP/Actions/Clear.hpp>
#include <HMP/Actions/Load.hpp>
#include <HMP/Actions/Delete.hpp>
#include <HMP/Actions/Extrude.hpp>
#include <HMP/Actions/MakeConforming.hpp>
#include <HMP/Actions/Paste.hpp>
#include <HMP/Actions/Project.hpp>
#include <HMP/Actions/Refine.hpp>
#include <HMP/Actions/Rotate.hpp>
#include <HMP/Actions/Transform.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/collections/Namer.hpp>
#include <string>

namespace HMP::Gui::HrDescriptions
{

	using DagNamer = cpputils::collections::Namer<const HMP::Dag::Node*>;
	using ProjectAction = Actions::Project<cinolib::Mesh_std_attributes, cinolib::Vert_std_attributes, cinolib::Edge_std_attributes, cinolib::Polygon_std_attributes>;

	std::string name(const HMP::Dag::Node& _node, DagNamer& _dagNamer);
	std::string describe(Meshing::ERefinementScheme _scheme);
	std::string describe(const Vec& _vec);
	std::string describe(const Mat4& _mat);
	std::string describe(const std::vector<unsigned int>& _ids);
	std::string describe(const HMP::Meshing::Utils::PolyVertLoc _loc);
	std::string describeFaces(Id _forwardFaceOffset, Id _upFaceOffset);
	std::string describe(const HMP::Dag::Delete& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Extrude& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Refine& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Delete& _operation, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Extrude& _operation, DagNamer& _dagNamer);
	std::string describe(const HMP::Dag::Refine& _operation, DagNamer& _dagNamer);

	std::string describe(const Actions::Clear& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Delete& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Extrude& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Load& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::MakeConforming& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::MoveVert& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Paste& _action, DagNamer& _dagNamer);
	std::string describe(const ProjectAction& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Refine& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Rotate& _action, DagNamer& _dagNamer);
	std::string describe(const Actions::Transform& _action, DagNamer& _dagNamer);
	std::string describe(const Commander::Action& _action, DagNamer& _dagNamer);

}