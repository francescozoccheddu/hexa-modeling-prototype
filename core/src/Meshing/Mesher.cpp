#include <HMP/Meshing/Mesher.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <cinolib/Moller_Trumbore_intersection.h>
#include <limits>
#include <cinolib/octree.h>
#include <queue>
#include <cassert>

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
		: MarkerSetBase{ _mesher }, m_data{}, HMP::Utils::ConstDerefRanged<std::unordered_set<const Dag::Element*>>{m_data}
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
		: MarkerSetBase{ _mesher }, m_data{}, HMP::Utils::ConstMapRanged<
		std::unordered_set < std::pair<const Dag::Element*, Id>, Internal::FaceMarkerHasher>,
		std::pair<const Dag::Element&, Id>,
		Internal::faceMarkerConvert
		>{ m_data }
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
		: m_mesh(), m_elementToPid{},
		m_polyMarkerSet{ *this }, m_faceMarkerSet{ *this },
		m_polyColor{ cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.35f) }, m_edgeColor{ cinolib::Color::BLACK() },
		m_dirty{ false }, m_visibleFaceIndices{}, m_visibleEdgeIndices{}, m_removedIds{},
		HMP::Utils::ConstAndNonConstMapRanged<
		std::unordered_map<Dag::Element*, Id>,
		std::pair<const Dag::Element&, Id>,
		Internal::mesherEntryConstConvert,
		std::pair<Dag::Element&, Id>,
		Internal::mesherEntryConvert
		>{m_elementToPid}
	{
		m_polyMarkerSet.color() = cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.5f);
		m_faceMarkerSet.color() = cinolib::Color::hsv2rgb(0.0f, 0.0f, 0.7f);
		m_removedIds.vids.reserve(8);
		m_removedIds.eids.reserve(12);
		m_removedIds.fids.reserve(6);
		m_mesh.draw_back_faces = false;
		m_mesh.show_mesh(true);
		m_mesh.show_mesh_flat();
		m_mesh.show_marked_face(true);
		m_mesh.show_out_wireframe_width(2.0f);
		updateColors();
	}

	void Mesher::paintEdge(Id _eid, const cinolib::Color& _color)
	{
		m_edgesPainted[toI(_eid)] = true;
		updateEdgeColor(_eid, _color);
	}

	void Mesher::unpaintEdge(Id _eid)
	{
		if (m_edgesPainted[toI(_eid)])
		{
			m_edgesPainted[toI(_eid)] = false;
			updateEdgeColor(_eid, m_edgeColor);
		}
	}

	void Mesher::updateEdgeColor(Id _eid, const cinolib::Color& _color)
	{
		if (m_mesh.edge_data(_eid).color != _color)
		{
			m_mesh.edge_data(_eid).color = _color;
			const Id visibleIndex{ m_visibleEdgeIndices[toI(_eid)] };
			if (visibleIndex != noId)
			{
				m_mesh.updateGL_out_e(_eid, visibleIndex);
			}
		}
	}

	const Mesher::Mesh& Mesher::mesh() const
	{
		return m_mesh;
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
		_element.pid = m_mesh.num_polys();
		onElementAdd(_element);
		const Id pid{ m_mesh.poly_add(cpputils::range::of(_element.vids).toVector()) };
		m_mesh.poly_data(pid).m_element = &_element;
		m_mesh.poly_data(pid).color = m_polyColor;
		m_edgesPainted.resize(toI(m_mesh.num_edges()), false);
		for (const Id eid : m_mesh.adj_p2e(pid))
		{
			if (!m_edgesPainted[toI(eid)])
			{
				m_mesh.edge_data(eid).color = m_edgeColor;
			}
		}
		m_elementToPid[&_element] = pid;
		m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = true;
		onElementAdded(_element);
	}

	Id Mesher::addVert(const Vec& _vert)
	{
		return m_mesh.vert_add(_vert);
	}

	void Mesher::remove(Dag::Element& _element, bool _removeVids)
	{
		const Id pid{ elementToPid(_element) };
		assert(pid != noId);
		m_mesh.poly_dangling_ids(pid, m_removedIds.vids, m_removedIds.eids, m_removedIds.fids);
		m_removedIds.pid = pid;
		m_removedIds.vidsActuallyRemoved = _removeVids;
		onElementRemove(_element, m_removedIds);
		m_elementToPid.erase(&_element);
		_element.pid = noId;
		for (const Id eid : m_removedIds.eids)
		{
			m_edgesPainted[toI(eid)] = m_edgesPainted.back();
			m_edgesPainted.pop_back();
		}
		const Id lastPid{ m_mesh.num_polys() - 1 };
		if (pid != lastPid)
		{
			m_elementToPid[&m_mesh.poly_data(lastPid).element()] = pid;
		}
		m_mesh.poly_disconnect(pid, m_removedIds.vids, m_removedIds.eids, m_removedIds.fids);
		for (const Id fid : m_removedIds.fids) m_mesh.face_remove_unreferenced(fid);
		for (const Id eid : m_removedIds.eids) m_mesh.edge_remove_unreferenced(eid);
		if (_removeVids)
		{
			for (const Id vid : m_removedIds.vids) m_mesh.vert_remove_unreferenced(vid);
		}
		m_mesh.poly_remove_unreferenced(pid);
		m_polyMarkerSet.m_data.erase(&_element);
		for (Id o{ 0 }; o < 6; o++)
		{
			m_faceMarkerSet.m_data.erase({ &_element, o });
		}
		m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = true;
		onElementRemoved(_element, m_removedIds);
	}

	void Mesher::moveVert(Id _vid, const Vec& _position)
	{
		if (m_mesh.vert(_vid) != _position)
		{
			m_mesh.vert(_vid) = _position;
			m_polyMarkerSet.m_dirty = m_faceMarkerSet.m_dirty = m_dirty = true;
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
			updateOctree();
			{
				m_visibleFaceIndices.resize(toI(m_mesh.num_faces()));
				Id lastI{};
				for (I fi{}; fi < m_visibleFaceIndices.size(); fi++)
				{
					const Id fid{ toId(fi) };
					Id pid;
					m_visibleFaceIndices[fi] = (m_mesh.face_is_on_srf(fid) && m_mesh.face_is_visible(fid, pid)) ? lastI++ : noId;
				}
			}
			{
				m_visibleEdgeIndices.resize(toI(m_mesh.num_edges()));
				Id lastI{};
				for (I ei{}; ei < m_visibleEdgeIndices.size(); ei++)
				{
					const Id eid{ toId(ei) };
					bool visible{ false };
					if (m_mesh.edge_is_on_srf(eid))
					{
						for (const Id pid : m_mesh.adj_e2p(eid))
						{
							if (!m_mesh.poly_data(pid).flags[cinolib::HIDDEN])
							{
								visible = true;
								break;
							}
						}
					}
					m_visibleEdgeIndices[ei] = visible ? lastI++ : noId;
				}
			}
			m_mesh.update_normals();
			m_mesh.update_bbox();
			m_mesh.updateGL_out();
			updateMeshMarkers();
			m_dirty = false;
		}
	}

	void Mesher::updateMeshTemp(const std::unordered_set<Id>& _changedVids)
	{
		if (m_dirty)
		{
			static constexpr double c_threshold{ 0.5 };
			if (_changedVids.size() >= static_cast<I>(toI(m_mesh.num_polys()) * c_threshold))
			{
				m_mesh.update_normals();
				m_mesh.updateGL_out();
			}
			else
			{
				std::unordered_set<Id> changedFids{}, changedEids{};
				for (const Id vid : _changedVids)
				{
					const std::vector<Id>& fids{ m_mesh.adj_v2f(vid) };
					changedFids.insert(fids.begin(), fids.end());
					const std::vector<Id>& eids{ m_mesh.adj_v2e(vid) };
					changedEids.insert(eids.begin(), eids.end());
				}
				for (const Id fid : changedFids)
				{
					m_mesh.update_f_normal(fid);
				}
				for (const Id vid : _changedVids)
				{
					m_mesh.update_v_normal(vid);
				}
				for (const Id fid : changedFids)
				{
					const Id visibleFaceId{ m_visibleFaceIndices[toI(fid)] };
					if (visibleFaceId != noId)
					{
						m_mesh.updateGL_out_f(fid, visibleFaceId * 2);
					}
				}
				for (const Id eid : changedEids)
				{
					const Id visibleEdgeId{ m_visibleEdgeIndices[toI(eid)] };
					if (visibleEdgeId != noId)
					{
						m_mesh.updateGL_out_e(eid, visibleEdgeId);
					}
				}
			}
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

	bool Mesher::pick(const Vec& _from, const Vec& _normDir, Id& _pid, Id& _fid, Id& _eid, Id& _vid, bool _allowBehind) const
	{
		double minT{ std::numeric_limits<double>::infinity() };
		_pid = _fid = _eid = _vid = noId;
		if (_allowBehind ? m_octree->intersects_line(_from, _normDir, minT, _fid) : m_octree->intersects_ray(_from, _normDir, minT, _fid))
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

	void Mesher::updateOctree()
	{
		m_octree = std::make_unique<cinolib::Octree>();
		for (Id fid{}; fid < m_mesh.num_faces(); fid++)
		{
			Id facePid;
			if (m_mesh.face_is_visible(fid, facePid))
			{
				const bool cw{ m_mesh.poly_face_is_CW(facePid, fid) };
				for (I ti{}; ti < 2; ti++)
				{
					m_octree->push_triangle(fid, {
							m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + (cw ? 2 : 0)]),
							m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + 1]),
							m_mesh.vert(m_mesh.face_tessellation(fid)[ti * 3 + (cw ? 0 : 2)]),
						}
					);
				}
			}
		}
		m_octree->build();
	}

}