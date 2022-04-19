#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Utils/span.hpp>
#include <stdexcept>

namespace HMP::Dag
{

	Element::Element()
		: Node{ EType::Element }
	{}

	unsigned int& Element::pid()
	{
		return m_pid;
	}

	unsigned int Element::pid() const
	{
		return m_pid;
	}

	std::array<cinolib::vec3d, 8>& Element::vertices()
	{
		return m_vertices;
	}

	const std::array<cinolib::vec3d, 8>& Element::vertices() const
	{
		return m_vertices;
	}

	Operation*& Element::parent()
	{
		if (parents().size() != 1)
		{
			throw std::logic_error{ "not one parent" };
		}
		return reinterpret_cast<Operation*&>(parents()[0]);
	}

	const Operation* Element::parent() const
	{
		return const_cast<Element*>(this)->parent();
	}

	std::span<Operation*> Element::children()
	{
		return Utils::span<Operation>(Node::children());
	}

	std::span<const Operation* const> Element::children() const
	{
		return Utils::constSpan<const Operation>(Node::children());
	}

}