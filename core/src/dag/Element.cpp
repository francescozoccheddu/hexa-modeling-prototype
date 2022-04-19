#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
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

	const Operation& Element::parent() const
	{
		if (parents().size() != 1)
		{
			throw std::logic_error{ "not one parent" };
		}
		return *m_parents.begin();
	}

	Utils::SetView<Operation>& Element::parents()
	{
		return m_parents;
	}

	const Utils::SetView<Operation>& Element::parents() const
	{
		return m_parents;
	}

	Utils::SetView<Operation>& Element::children()
	{
		return m_children;
	}

	const Utils::SetView<Operation>& Element::children() const
	{
		return m_children;
	}

}