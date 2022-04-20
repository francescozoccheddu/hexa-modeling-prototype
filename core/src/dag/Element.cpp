#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <stdexcept>
#include <algorithm>

namespace HMP::Dag
{

	Element::Element()
		: Node{ EType::Element }
	{
		m_vertices.fill(cinolib::vec3d{ 0,0,0 });
	}

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

	Element::DagSetView& Element::parents()
	{
		return m_parents;
	}

	const Element::DagSetView& Element::parents() const
	{
		return m_parents;
	}

	Element::DagSetView& Element::children()
	{
		return m_children;
	}

	const Element::DagSetView& Element::children() const
	{
		return m_children;
	}

}