#include <hexa-modeling-prototype/gui/dag/treeLayout.hpp>

#include <stdexcept>

namespace HMP::Gui::Dag
{

	using Real = Layout::Real;
	using Point = Layout::Point;
	using Line = Layout::Line;
	using Node = Layout::Node;

	constexpr Real c_nodeSpacingX{ 0.1 };
	constexpr Real c_nodeSpacingY{ 1.0 };
	constexpr Real c_nodeRadius{ 0.5 };
	constexpr Real c_lineThickness{ 0.025 };

	struct ProtoNode final
	{

		ProtoNode* pUp{ nullptr };
		ProtoNode* pLeft{ nullptr }, * pRight{ nullptr };
		std::vector<ProtoNode*> down{};
		Node::EType type;
		Real x{};

	};

	Layout treeLayout(const OperationsTree& _tree)
	{
		throw std::runtime_error{ "not implemented" };
	}

}