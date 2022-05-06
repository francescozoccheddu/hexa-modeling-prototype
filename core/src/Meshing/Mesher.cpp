#include <HMP/Meshing/Mesher.hpp>

#include <cpputils/collections/conversions.hpp>

namespace HMP::Meshing
{

	// Mesher::PolyAttributes

	Dag::Element& Mesher::PolyAttributes::element()
	{
		return *m_element;
	}

	const Dag::Element& Mesher::PolyAttributes::element() const
	{
		return *m_element;
	}

	// Mesher

	Id Mesher::getOrAddVert(const Vec& _vert)
	{
		Id vid;
		if (!getVert(_vert, vid))
		{
			vid = m_mesh.vert_add(_vert);
		}
		return vid;
	}

	Mesher::Mesher()
		: m_mesh(), m_elementToPid{}, Internal::ElementToPidIterable{ m_elementToPid }
	{
	}

	const Mesher::Mesh& Mesher::mesh() const
	{
		return m_mesh;
	}

	bool Mesher::getVert(const Vec& _vert, Id& _vid) const
	{
		constexpr Real c_maxVertDistance{ 1e-6 };
		if (!m_mesh.num_verts())
		{
			return false;
		}
		_vid = m_mesh.pick_vert(_vert);
		return m_mesh.vert(_vid).dist(_vert) <= c_maxVertDistance;
	}

	Id Mesher::getVert(const Vec& _vert) const
	{
		Id vid;
		if (!getVert(_vert, vid))
		{
			throw std::logic_error{ "not found" };
		}
		return vid;
	}

	bool Mesher::hasVert(const Vec& _vert) const
	{
		Id vid;
		return getVert(_vert, vid);
	}

	bool Mesher::has(const Dag::Element& _element) const
	{
		return m_elementToPid.contains(const_cast<Dag::Element*>(&_element));
	}

	Id Mesher::elementToPid(const Dag::Element& _element) const
	{
		return m_elementToPid.at(const_cast<Dag::Element*>(&_element));
	}

	Dag::Element& Mesher::pidToElement(Id _pid)
	{
		return m_mesh.poly_data(_pid).element();
	}

	const Dag::Element& Mesher::pidToElement(Id _pid) const
	{
		return const_cast<Mesher*>(this)->pidToElement(_pid);
	}

	void Mesher::add(Dag::Element& _element)
	{
		PolyVertIds vids;
		for (std::size_t i{ 0 }; i < 8; i++)
		{
			vids[i] = getOrAddVert(_element.vertices()[i]);
		}
		const Id pid{ m_mesh.poly_add(cpputils::collections::conversions::toVector(vids)) };
		m_mesh.poly_data(pid).m_element = &_element;
		m_elementToPid[&_element] = pid;
		m_mesh.updateGL();
	}

	void Mesher::remove(Dag::Element& _element)
	{
		const Id pid{ elementToPid(_element) };
		m_elementToPid.erase(&_element);
		const Id lastPid{ m_mesh.num_polys() - 1 };
		if (pid != lastPid)
		{
			m_elementToPid[&m_mesh.poly_data(lastPid).element()] = pid;
		}
		m_mesh.poly_remove(pid, true);
		m_mesh.updateGL();
	}

	void Mesher::moveVert(Id _vid, const Vec& _position)
	{
		m_mesh.vert(_vid) = _position;
		m_mesh.updateGL();
	}

	void Mesher::clear()
	{
		m_mesh.clear();
		m_mesh.updateGL();
		m_elementToPid.clear();
	}

}