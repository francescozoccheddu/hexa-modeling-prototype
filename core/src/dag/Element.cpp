#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <stdexcept>
#include <algorithm>

namespace HMP::Dag
{

	Element::Element()
		: Node{ EType::Element }
	{
		m_vertices.fill(Vec{ 0,0,0 });
	}

	Id& Element::pid()
	{
		return m_pid;
	}

	Id Element::pid() const
	{
		return m_pid;
	}

	PolyVerts& Element::vertices()
	{
		return m_vertices;
	}

	const PolyVerts& Element::vertices() const
	{
		return m_vertices;
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