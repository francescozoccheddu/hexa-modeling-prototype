#include <HMP/Dag/Element.hpp>

#include <stdexcept>

namespace HMP::Dag
{

	Element::Element()
		: Node{ EType::Element }, m_parents{ parentsHandle() }, m_children{ childrenHandle() }, vids{}, pid{ noId }
	{
		m_vertices.fill(Vec{ 0,0,0 });
		vids.fill(noId);
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

	FaceVertIds Element::faceVids(I _faceOffset) const
	{
		switch (_faceOffset)
		{
			case 0:
				return { vids[0], vids[1], vids[2], vids[3] };
			case 1:
				return { vids[4], vids[7], vids[6], vids[5] };
			case 2:
				return { vids[1], vids[5], vids[6], vids[2] };
			case 3:
				return { vids[0], vids[3], vids[7], vids[4] };
			case 4:
				return { vids[0], vids[4], vids[5], vids[1] };
			case 5:
				return { vids[3], vids[2], vids[6], vids[7] };
			default:
				throw std::range_error{ "out of range" };
		}
	}


}