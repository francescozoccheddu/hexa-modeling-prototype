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
		const Id vid{ getVert(_vert) };
		if (vid == noId)
		{
			return m_mesh.vert_add(_vert);
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

	Id Mesher::getVert(const Vec& _vert) const
	{
		constexpr Real c_maxVertDistance{ 1e-6 };
		if (!m_mesh.num_verts())
		{
			return noId;
		}
		const Id vid{ m_mesh.pick_vert(_vert) };
		return m_mesh.vert(vid).dist(_vert) <= c_maxVertDistance ? vid : noId;
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
		const Id oldVid{ getVert(_position) };
		if (oldVid != noId && oldVid != _vid)
		{
			throw std::logic_error{ "move will result in merge" };
		}
		m_mesh.vert(_vid) = _position;
		for (const Id pid : m_mesh.adj_v2p(_vid))
		{
			pidToElement(pid).vertices()[m_mesh.poly_vert_offset(pid, _vid)] = _position;
		}
		m_mesh.updateGL();
	}

	void Mesher::clear()
	{
		m_mesh.clear();
		m_mesh.updateGL();
		m_elementToPid.clear();
	}

}