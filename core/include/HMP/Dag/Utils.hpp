#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Node.hpp>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <functional>
#include <HMP/Utils/Serialization.hpp>

namespace HMP::Dag::Utils
{

	std::vector<Node*> descendants(Node& _node, std::function<bool(const Node&)> _branchSelector = [](const Node&) { return true; });
	std::vector<const Node*> descendants(const Node& _node, std::function<bool(const Node&)> _branchSelector = [](const Node&) { return true; });

	void serialize(HMP::Utils::Serialization::Serializer& _serializer, const Node& _node);
	Node& deserialize(HMP::Utils::Serialization::Deserializer& _deserializer);

	void transform(Node& _root, const Mat4& _transform);
	void transform(Node& _root, const Mat3& _transform);
	Node& cloneNode(const Node& _node);
	Node& clone(const Node& _root);

}