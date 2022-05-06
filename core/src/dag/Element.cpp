#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <stdexcept>
#include <algorithm>

namespace HMP::Dag
{

	Element::Element()
		: Node{ EType::Element }, m_parents{ parentsHandle() }, m_children{ childrenHandle() }
	{
		m_vertices.fill(Vec{ 0,0,0 });
	}

	PolyVerts& Element::vertices()
	{
		return m_vertices;
	}

	const PolyVerts& Element::vertices() const
	{
		return m_vertices;
	}

	Element::Set& Element::forward(bool _descending)
	{
		return _descending ? m_children : m_parents;
	}

	const Element::Set& Element::forward(bool _descending) const
	{
		return const_cast<Element*>(this)->forward(_descending);
	}

	Element::Set& Element::back(bool _descending)
	{
		return forward(!_descending);
	}

	const Element::Set& Element::back(bool _descending) const
	{
		return const_cast<Element*>(this)->back(_descending);
	}

	Element::Set& Element::parents()
	{
		return m_parents;
	}

	const Element::Set& Element::parents() const
	{
		return m_parents;
	}

	Element::Set& Element::children()
	{
		return m_children;
	}

	const Element::Set& Element::children() const
	{
		return m_children;
	}

}