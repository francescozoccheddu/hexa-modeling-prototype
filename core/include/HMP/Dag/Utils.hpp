#pragma once

#include <HMP/Dag/Node.hpp>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <functional>

namespace HMP::Dag::Utils
{

	std::vector<Node*> descendants(Node& _node, std::function<bool(const Node&)> _branchSelector = [](const Node&) { return true; });
	std::vector<const Node*> descendants(const Node& _node, std::function<bool(const Node&)> _branchSelector = [](const Node&) { return true; });

	std::ostream& operator<<(std::ostream& _stream, const Node& _node);
	std::istream& operator>>(std::istream& _stream, Node*& _node);

}