#include <HMP/Meshing/Mesher.hpp>

#include <HMP/Utils/Collections.hpp>

namespace HMP::Meshing
{

	// Mesher::PolyAttributes

	const Dag::Element& Mesher::PolyAttributes::element() const
	{
		return *m_element;
	}

	// Mesher::ProjectClient

	void Mesher::ProjectClient::apply(Mesher& _mesher, Dag::Operation& _operation)
	{
		_mesher.apply(_operation);
	}

	void Mesher::ProjectClient::unapply(Mesher& _mesher, Dag::Operation& _operation)
	{
		_mesher.unapply(_operation);
	}

	void Mesher::ProjectClient::clear(Mesher& _mesher)
	{
		_mesher.clear();
	}

	// Mesher


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

	Id Mesher::getOrAddVert(const Vec& _vert)
	{
		Id vid;
		if (!getVert(_vert, vid))
		{
			vid = m_mesh.vert_add(_vert);
		}
		return vid;
	}

	bool Mesher::hasVert(const Vec& _vert) const
	{
		Id vid;
		return getVert(_vert, vid);
	}

	void Mesher::addPoly(Dag::Element& _element)
	{
		const Id pid{ m_mesh.num_polys() - 1 };
		events.polyAdding.invoker(pid);
		PolyVertIds vids;
		for (std::size_t i{ 0 }; i < 8; i++)
		{
			vids[i] = getOrAddVert(_element.vertices()[i]);
		}
		m_mesh.poly_add(Utils::Collections::toVector(vids));
		m_elementToPid[&_element] = pid;
		events.polyAdded.invoker(pid);
	}

	void Mesher::removePoly(Dag::Element& _element)
	{
		const Id pid{ elementToPid(_element) };
		const Id lastPid{ m_mesh.num_polys() - 1 };
		events.polyRemoving.invoker(pid);
		if (pid != lastPid)
		{
			events.polyIdChanging.invoker(lastPid, pid);
			m_elementToPid[&m_mesh.poly_data(lastPid).element()] = pid;
		}
		m_mesh.poly_remove(pid, true);
		events.polyRemoved.invoker(pid);
		if (pid != lastPid)
		{
			events.polyIdChanged.invoker(lastPid, pid);
		}
	}

	void Mesher::apply(Dag::Operation& _operation)
	{
		for (Dag::Element& parent : _operation.parents())
		{
			removePoly(parent);
		}
		for (Dag::Element& child : _operation.children())
		{
			addPoly(child);
		}
	}

	void Mesher::unapply(Dag::Operation& _operation)
	{
		for (Dag::Element& child : _operation.children())
		{
			removePoly(child);
		}
		for (Dag::Element& parent : _operation.parents())
		{
			addPoly(parent);
		}
	}

	void Mesher::clear()
	{
		events.clearing.invoker();
		m_mesh.clear();
		m_elementToPid.clear();
		events.cleared.invoker();
	}

	const Mesher::Mesh& Mesher::mesh() const
	{
		return m_mesh;
	}

	bool& Mesher::pidMarked(Id _pid)
	{
		return m_mesh.poly_data(_pid).flags[cinolib::MARKED];
	}

	bool Mesher::pidMarked(Id _pid) const
	{
		return const_cast<Mesher*>(this)->pidMarked(_pid);
	}

	bool& Mesher::fidMarked(Id _fid)
	{
		return m_mesh.face_data(_fid).flags[cinolib::MARKED];
	}

	bool Mesher::fidMarked(Id _fid) const
	{
		return const_cast<Mesher*>(this)->fidMarked(_fid);
	}

	bool Mesher::isElementActive(const Dag::Element& _element) const
	{
		return m_elementToPid.contains(&_element);
	}

	Id Mesher::elementToPid(const Dag::Element& _element) const
	{
		return m_elementToPid.at(&_element);
	}

}