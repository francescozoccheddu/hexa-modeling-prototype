#include <HMP/Meshing/Mesher.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <cinolib/Moller_Trumbore_intersection.h>
#include <limits>

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

	// Mesher::MarkerSetBase

	Mesher::MarkerSetBase::MarkerSetBase(Mesher& _mesher)
		: m_mesher{ _mesher }, m_dirty{ false }, m_color{ cinolib::Color::WHITE() }
	{}

	cinolib::Color& Mesher::MarkerSetBase::color()
	{
		return m_color;
	}

	const cinolib::Color& Mesher::MarkerSetBase::color() const
	{
		return m_color;
	}

	void Mesher::MarkerSetBase::requestUpdate()
	{
		m_dirty = true;
	}

	// Mesher::PolyMarkerSet

	Mesher::PolyMarkerSet::PolyMarkerSet(Mesher& _mesher)
		: MarkerSetBase{ _mesher }, m_data{}, Internal::PolyMarkerIterable{ m_data }
	{}

	void Mesher::PolyMarkerSet::mark(const Dag::Element& _element, bool _marked)
	{
		const Id pid{ m_mesher.elementToPid(_element) };
		std::bitset<8>& flags{ m_mesher.m_mesh.poly_data(pid).flags };
		if (flags[cinolib::MARKED] != _marked)
		{
			flags[cinolib::MARKED] = _marked;
			m_dirty = true;
		}
	}

	bool Mesher::PolyMarkerSet::has(const Dag::Element& _element) const
	{
		return m_data.contains(&_element);
	}

	bool Mesher::PolyMarkerSet::add(const Dag::Element& _element)
	{
		if (m_data.insert(&_element).second)
		{
			mark(_element, true);
			return true;
		}
		return false;
	}

	bool Mesher::PolyMarkerSet::remove(const Dag::Element& _element)
	{
		if (m_data.erase(&_element))
		{
			mark(_element, false);
			return true;
		}
		return false;
	}

	bool Mesher::PolyMarkerSet::clear()
	{
		const bool wasEmpty{ empty() };
		for (const Dag::Element& element : *this)
		{
			mark(element, false);
		}
		m_data.clear();
		return !wasEmpty;
	}

	Mesher::FaceMarkerSet::FaceMarkerSet(Mesher& _mesher)
		: MarkerSetBase{ _mesher }, m_data{}, Internal::FaceMarkerIterable{ m_data }
	{}

	void Mesher::FaceMarkerSet::mark(const Dag::Element& _element, Id _faceOffset, bool _marked)
	{
		const Id pid{ m_mesher.elementToPid(_element) };
		const Id fid{ m_mesher.m_mesh.poly_face_id(pid, _faceOffset) };
		std::bitset<8>& flags{ m_mesher.m_mesh.face_data(fid).flags };
		if (flags[cinolib::MARKED] != _marked)
		{
			flags[cinolib::MARKED] = _marked;
			m_dirty = true;
		}
	}

	bool Mesher::FaceMarkerSet::has(const Dag::Element& _element, Id _faceOffset) const
	{
		return m_data.contains({ &_element, _faceOffset });
	}

	bool Mesher::FaceMarkerSet::addAll(const Dag::Element& _element)
	{
		bool added{ false };
		for (Id o{ 0 }; o < 6; o++)
		{
			added |= add(_element, o);
		}
		return added;
	}

	bool Mesher::FaceMarkerSet::add(const Dag::Element& _element, Id _faceOffset)
	{
		if (m_data.insert({ &_element, _faceOffset }).second)
		{
			mark(_element, _faceOffset, true);
			return true;
		}
		return false;
	}

	bool Mesher::FaceMarkerSet::removeAll(const Dag::Element& _element)
	{
		bool removed{ false };
		for (Id o{ 0 }; o < 6; o++)
		{
			removed |= remove(_element, o);
		}
		return removed;
	}

	bool Mesher::FaceMarkerSet::remove(const Dag::Element& _element, Id _faceOffset)
	{
		if (m_data.erase({ &_element, _faceOffset }))
		{
			mark(_element, _faceOffset, false);
			return true;
		}
		return false;
	}

	bool Mesher::FaceMarkerSet::clear()
	{
		const bool wasEmpty{ empty() };
		for (const auto& [element, faceOffset] : *this)
		{
			mark(element, faceOffset, false);
		}
		m_data.clear();
		return !wasEmpty;
	}

	// Mesher

	Id Mesher::getOrAddVert(const Vec& _vert)
	{
		const Id vid{ getVert(_vert) };
		if (vid == noId)
		{
			m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = true;
			return m_mesh.vert_add(_vert);
		}
		return vid;
	}

	Mesher::Mesher()
		: m_mesh(), m_elementToPid{}, Internal::ElementToPidIterable{ m_elementToPid },
		m_polyMarkerSet{ *this }, m_faceMarkerSet{ *this },
		m_polyColor{ cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.35f) }, m_edgeColor{ cinolib::Color::BLACK() },
		m_dirty{ false }
	{
		m_polyMarkerSet.color() = cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.5f);
		m_faceMarkerSet.color() = cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.7f);
		m_mesh.draw_back_faces = false;
		m_mesh.show_mesh(true);
		m_mesh.show_mesh_flat();
		m_mesh.show_marked_face(true);
		m_mesh.show_out_wireframe_width(2.0f);
		updateColors();
	}

	const Mesher::Mesh& Mesher::mesh() const
	{
		return m_mesh;
	}

	bool Mesher::areVerticesCollidingAt(const Vec& _vert) const
	{
		if (!m_mesh.num_verts())
		{
			return true;
		}
		bool pickedSome{ false };
		for (Id vid{}; vid < m_mesh.num_verts(); vid++)
		{
			if (m_mesh.vert(vid).dist(_vert) < c_maxVertDistance)
			{
				if (pickedSome)
				{
					return true;
				}
				pickedSome = true;
			}
		}
		return false;
	}

	Id Mesher::getVert(const Vec& _vert) const
	{
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
		auto it{ m_elementToPid.find(const_cast<Dag::Element*>(&_element)) };
		return (it != m_elementToPid.end()) ? it->second : noId;
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
		//vids = Utils::sortVids(m_mesh, vids);
		const Id pid{ m_mesh.poly_add(cpputils::collections::conversions::toVector(vids)) };
		m_mesh.poly_data(pid).m_element = &_element;
		m_mesh.poly_data(pid).color = m_polyColor;
		for (const Id eid : m_mesh.adj_p2e(pid))
		{
			m_mesh.edge_data(eid).color = m_edgeColor;
		}
		m_elementToPid[&_element] = pid;
		m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = true;
	}

	void Mesher::remove(Dag::Element& _element)
	{
		const Id pid{ elementToPid(_element) };
		if (pid == noId)
		{
			throw std::logic_error{ "not an element" };
		}
		onElementRemove(_element);
		m_elementToPid.erase(&_element);
		const Id lastPid{ m_mesh.num_polys() - 1 };
		if (pid != lastPid)
		{
			m_elementToPid[&m_mesh.poly_data(lastPid).element()] = pid;
		}
		m_mesh.poly_remove(pid, true);
		m_polyMarkerSet.m_data.erase(&_element);
		for (Id o{ 0 }; o < 6; o++)
		{
			m_faceMarkerSet.m_data.erase({ &_element, o });
		}
		m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = true;
		onElementRemoved(_element);
	}

	void Mesher::moveVert(Id _vid, const Vec& _position)
	{
		const Id oldVid{ getVert(_position) };
		if (oldVid != noId && oldVid != _vid)
		{
			throw std::logic_error{ "move would result in merge" };
		}
		if (m_mesh.vert(_vid) != _position)
		{
			m_mesh.vert(_vid) = _position;
			for (const Id pid : m_mesh.adj_v2p(_vid))
			{
				pidToElement(pid).vertices()[m_mesh.poly_vert_offset(pid, _vid)] = _position;
			}
			m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = true;
		}
	}

	bool Mesher::tryMoveVerts(const std::unordered_map<Id, Vec>& _verts)
	{
		std::vector<std::pair<Id, Vec>> backup{};
		backup.reserve(_verts.size());
		for (const auto& [vid, pos] : _verts)
		{
			backup.push_back({ vid, m_mesh.vert(vid) });
			m_mesh.vert(vid) = pos;
		}
		bool ok{ true };
		for (const auto& [vid, pos] : _verts)
		{
			if (areVerticesCollidingAt(pos))
			{
				ok = false;
				break;
			}
		}
		if (!ok)
		{
			for (const auto& [vid, oldPos] : backup)
			{
				m_mesh.vert(vid) = oldPos;
			}
		}
		if (ok && !_verts.empty())
		{
			for (const auto& [vid, pos] : _verts)
			{
				for (const Id pid : m_mesh.adj_v2p(vid))
				{
					pidToElement(pid).vertices()[m_mesh.poly_vert_offset(pid, vid)] = pos;
				}
			}
			m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = true;
		}
		return ok;
	}

	void Mesher::moveVerts(const std::unordered_map<Id, Vec>& _verts)
	{
		if (!tryMoveVerts(_verts))
		{
			throw std::logic_error{ "move would result in merge" };
		}
	}

	void Mesher::clear()
	{
		onClear();
		m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = m_mesh.num_polys();
		m_mesh.clear();
		m_elementToPid.clear();
		m_polyMarkerSet.m_data.clear();
		m_faceMarkerSet.m_data.clear();
		onCleared();
	}

	Mesher::PolyMarkerSet& Mesher::polyMarkerSet()
	{
		return m_polyMarkerSet;
	}

	const Mesher::PolyMarkerSet& Mesher::polyMarkerSet() const
	{
		return m_polyMarkerSet;
	}

	Mesher::FaceMarkerSet& Mesher::faceMarkerSet()
	{
		return m_faceMarkerSet;
	}

	const Mesher::FaceMarkerSet& Mesher::faceMarkerSet() const
	{
		return m_faceMarkerSet;
	}

	cinolib::Color& Mesher::polyColor()
	{
		return m_polyColor;
	}

	const cinolib::Color& Mesher::polyColor() const
	{
		return m_polyColor;
	}

	cinolib::Color& Mesher::edgeColor()
	{
		return m_edgeColor;
	}

	const cinolib::Color& Mesher::edgeColor() const
	{
		return m_edgeColor;
	}

	void Mesher::updateColors(bool _poly, bool _edge)
	{
		if (_poly)
		{
			m_mesh.poly_set_color(m_polyColor);
		}
		if (_edge)
		{
			m_mesh.edge_set_color(m_edgeColor);
		}
	}

	void Mesher::updateMesh()
	{
		if (m_dirty)
		{
			m_mesh.update_normals();
			//m_mesh.updateGL_in();
			m_mesh.update_bbox();
			m_mesh.updateGL_out();
			updateMeshMarkers();
			m_dirty = false;
		}
	}

	void Mesher::updateMeshMarkers()
	{
		if (m_polyMarkerSet.m_dirty || m_faceMarkerSet.m_dirty)
		{
			m_mesh.marked_face_color = m_faceMarkerSet.color();
			m_mesh.marked_poly_color = m_polyMarkerSet.color();
			m_mesh.updateGL_marked();
			m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = false;
		}
	}

	bool Mesher::pick(const Vec& _from, const Vec& _normDir, Id& _pid, Id& _fid, Id& _eid, Id& _vid) const
	{
		double minT{ std::numeric_limits<double>::infinity() };
		_fid = noId;
		for (std::size_t fid{}; fid < m_mesh.num_faces(); fid++)
		{
			Id facePid;
			if (m_mesh.face_is_visible(fid, facePid))
			{
				const bool cw{ m_mesh.poly_face_is_CW(facePid, fid) };
				for (std::size_t ti{}; ti < 2; ti++)
				{
					bool back, coplanar;
					double t;
					Vec bary;
					if (cinolib::Moller_Trumbore_intersection(
						_from,
						_normDir,
						m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + (cw ? 2 : 0)]),
						m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + 1]),
						m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + (cw ? 0 : 2)]),
						back,
						coplanar,
						t,
						bary))
					{
						if (!back && !coplanar && t < minT)
						{
							minT = t;
							_fid = fid;
						}
					}
				}
			}
		}
		if (_fid != noId)
		{
			const Vec point{ _from + _normDir * minT };
			_pid = m_mesh.adj_f2p(_fid)[0];
			_eid = Meshing::Utils::closestFaceEid(m_mesh, _fid, point);
			_vid = Meshing::Utils::closestFaceVid(m_mesh, _fid, point);
			return true;
		}
		else
		{
			return false;
		}
	}

}