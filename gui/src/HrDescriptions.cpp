#include <HMP/Gui/HrDescriptions.hpp>

#include <sstream>
#include <iomanip>

namespace HMP::Gui::HrDescriptions
{

	std::string name(const HMP::Dag::Node& _node, DagNamer& _dagNamer)
	{
		switch (_node.type())
		{
			case HMP::Dag::Node::EType::Element:
				return _dagNamer(&_node);
			case HMP::Dag::Node::EType::Operation:
				switch (_node.operation().primitive())
				{
					case HMP::Dag::Operation::EPrimitive::Delete:
						return "D-" + _dagNamer(&_node);
					case HMP::Dag::Operation::EPrimitive::Extrude:
						return "E-" + _dagNamer(&_node);
					case HMP::Dag::Operation::EPrimitive::Refine:
						return "R-" + _dagNamer(&_node);
				}
			default:
				return "?-" + _dagNamer(&_node);
		}
	}

	std::string describe(Meshing::ERefinementScheme _scheme)
	{
		switch (_scheme)
		{
			case Meshing::ERefinementScheme::Inset:
				return "Inset";
			case Meshing::ERefinementScheme::Subdivide3x3:
				return "Subdivide3x3";
			case Meshing::ERefinementScheme::AdapterEdgeSubdivide3x3:
				return "AdapterEdgeSubdivide3x3";
			case Meshing::ERefinementScheme::AdapterFaceSubdivide3x3:
				return "AdapterFaceSubdivide3x3";
			default:
				return "Unknown";
		}
	}

	std::string describe(const Vec& _vec)
	{
		std::ostringstream stream{};
		stream
			<< "{"
			<< std::fixed << std::setprecision(3)
			<< _vec.x() << ","
			<< _vec.y() << ","
			<< _vec.z()
			<< "}";
		return stream.str();
	}

	std::string describeFaces(Id _forwardFaceOffset, Id _upFaceOffset)
	{
		std::ostringstream stream{};
		stream << _forwardFaceOffset << "-" << _upFaceOffset;
		return stream.str();
	}

	std::string describe(const HMP::Dag::Delete& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Delete"
			<< " " << name(_element, _dagNamer)
			<< " (" << name(_operation, _dagNamer) << ")";
		return stream.str();
	}

	std::string describe(const HMP::Dag::Extrude& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Extrude"
			<< " " << name(_element, _dagNamer)
			<< " towards " << describeFaces(_operation.forwardFaceOffset(), _operation.upFaceOffset())
			<< " into " << name(_operation.children().single(), _dagNamer)
			<< " (" << name(_operation, _dagNamer) << ")";
		return stream.str();
	}

	std::string describe(const HMP::Dag::Refine& _operation, const HMP::Dag::Element& _element, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Refine"
			<< " " << name(_element, _dagNamer)
			<< " with scheme " << describe(_operation.scheme())
			<< " towards " << describeFaces(_operation.forwardFaceOffset(), _operation.upFaceOffset())
			<< " (" << name(_operation, _dagNamer) << ")";
		return stream.str();
	}

	std::string describe(const HMP::Dag::Delete& _operation, DagNamer& _dagNamer)
	{
		return describe(_operation, _operation.parents().single(), _dagNamer);
	}

	std::string describe(const HMP::Dag::Extrude& _operation, DagNamer& _dagNamer)
	{
		return describe(_operation, _operation.parents().single(), _dagNamer);
	}

	std::string describe(const HMP::Dag::Refine& _operation, DagNamer& _dagNamer)
	{
		return describe(_operation, _operation.parents().single(), _dagNamer);
	}

	std::string describe(const Actions::Clear& _action, DagNamer& _dagNamer)
	{
		return "Clear";
	}

	std::string describe(const Actions::Delete& _action, DagNamer& _dagNamer)
	{
		return describe(_action.operation(), _action.element(), _dagNamer);
	}

	std::string describe(const Actions::Extrude& _action, DagNamer& _dagNamer)
	{
		return describe(_action.operation(), _action.element(), _dagNamer);
	}

	std::string describe(const Actions::Load& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Load"
			<< " " << name(_action.newRoot(), _dagNamer);
		return stream.str();
	}

	std::string describe(const Actions::MakeConforming& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream << "Make conforming";
		stream << " (";
		std::vector<std::pair<const HMP::Dag::Refine*, const HMP::Dag::Element*>> operations{ _action.operations() };
		if (operations.empty())
		{
			stream << "no operations";
		}
		if (operations.size() > 5)
		{
			stream << operations.size() << " operations";
		}
		else
		{
			bool first{ true };
			for (const auto& [operation, element] : operations)
			{
				if (!first)
				{
					stream << ", ";
				}
				first = false;
				stream << name(*operation, _dagNamer) << " of " << name(*element, _dagNamer);
			}
		}
		stream << ")";
		return stream.str();
	}

	std::string describe(const Actions::MoveVert& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Move vertex"
			<< " " << _action.vertOffset()
			<< " of " << name(_action.element(), _dagNamer)
			<< " to " << describe(_action.position());
		return stream.str();
	}

	std::string describe(const Actions::Paste& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Paste"
			<< " at " << name(_action.element(), _dagNamer)
			<< " ("
			<< name(_action.operation(), _dagNamer)
			<< " towards " << describeFaces(_action.operation().forwardFaceOffset(), _action.operation().upFaceOffset())
			<< " into " << name(_action.operation().children().single(), _dagNamer)
			<< ")";
		return stream.str();
	}

	std::string describe(const ProjectAction& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Project to target"
			<< " " << &_action.target();
		return stream.str();
	}

	std::string describe(const Actions::Refine& _action, DagNamer& _dagNamer)
	{
		return describe(_action.operation(), _action.element(), _dagNamer);
	}

	std::string describe(const Actions::Rotate& _action, DagNamer& _dagNamer)
	{
		std::ostringstream stream{};
		stream
			<< "Rotate"
			<< " " << name(_action.operation().children().single(), _dagNamer)
			<< " (" << name(_action.operation(), _dagNamer) << ")";
		return stream.str();
	}

	std::string describe(const Commander::Action& _action, DagNamer& _dagNamer)
	{
		if (const Actions::Clear* action{ dynamic_cast<const Actions::Clear*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Delete* action{ dynamic_cast<const Actions::Delete*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Extrude* action{ dynamic_cast<const Actions::Extrude*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Load* action{ dynamic_cast<const Actions::Load*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::MakeConforming* action{ dynamic_cast<const Actions::MakeConforming*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::MoveVert* action{ dynamic_cast<const Actions::MoveVert*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Paste* action{ dynamic_cast<const Actions::Paste*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const ProjectAction* action{ dynamic_cast<const ProjectAction*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Refine* action{ dynamic_cast<const Actions::Refine*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		if (const Actions::Rotate* action{ dynamic_cast<const Actions::Rotate*>(&_action) }; action)
		{
			return describe(*action, _dagNamer);
		}
		return "Unknown action";
	}

}