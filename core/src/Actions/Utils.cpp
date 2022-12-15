#include <HMP/Actions/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <cpputils/collections/zip.hpp>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <algorithm>

namespace HMP::Actions::Utils
{

	Dag::Refine& prepareRefine(Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme, I _depth)
	{
		if (_depth < 1 || _depth > 3)
		{
			throw std::logic_error{ "depth must fall in range [1,3]" };
		}
		Dag::Refine& refine{ *new Dag::Refine{} };
		refine.scheme() = _scheme;
		refine.forwardFaceOffset() = _forwardFaceOffset;
		refine.upFaceOffset() = _upFaceOffset;
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(_scheme) };
		for (I i{ 0 }; i < refinement.polyCount(); i++)
		{
			Dag::Element& child{ *new Dag::Element{} };
			if (_depth > 1)
			{
				child.children().attach(prepareRefine(_forwardFaceOffset, _upFaceOffset, _scheme, _depth - 1));
			}
			refine.children().attach(child);
		}
		return refine;
	}

	std::vector<PolyVerts> previewRefine(const Meshing::Mesher& _mesher, const Dag::Refine& _refine)
	{
		const Dag::Element& element{ _refine.parents().single() };
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(_refine.scheme()) };
		if (refinement.polyCount() != _refine.children().size())
		{
			throw std::logic_error{ "wrong number of children" };
		}
		const Id pid{ _mesher.elementToPid(element) };
		if (pid == noId)
		{
			throw std::logic_error{ "not an element" };
		}
		const Id forwardFid{ mesh.poly_face_id(pid, _refine.forwardFaceOffset()) };
		const Id upFid{ mesh.poly_face_id(pid, _refine.upFaceOffset()) };
		const Id upEid{ mesh.face_shared_edge(forwardFid, upFid) };
		const PolyVertIds sourceVids{ Meshing::Utils::polyVids(mesh, pid, forwardFid, upEid) };
		const PolyVerts source{ Meshing::Utils::verts(mesh, sourceVids) };
		const std::vector<PolyVerts> polys{ refinement.apply(source) };
		return polys;
	}

	void applyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const std::vector<PolyVerts> polys{ previewRefine(_mesher, _refine) };
		for (const auto& [child, verts] : cpputils::collections::zip(_refine.children(), polys))
		{
			child.vertices() = verts;
			_mesher.add(child);
		}
		_mesher.remove(_refine.parents().single());
	}

	void applyRefineRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		applyRefine(_mesher, _refine);
		for (Dag::Element& child : _refine.children())
		{
			const auto refineIt{ child.children().singleIt([](const Dag::Operation& _op) {return _op.primitive() == Dag::Operation::EPrimitive::Refine; }) };
			if (refineIt != child.children().end())
			{
				applyRefineRecursive(_mesher, static_cast<Dag::Refine&>(*refineIt));
			}
		}
	}

	void unapplyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine, bool _detach)
	{
		for (Dag::Element& child : _refine.children())
		{
			_mesher.remove(child);
		}
		_mesher.add(_refine.parents().single());
		if (_detach)
		{
			_refine.parents().detachAll(false);
		}
	}

	void unapplyRefineRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine, bool _detach)
	{
		for (Dag::Element& child : _refine.children())
		{
			const auto refineIt{ child.children().singleIt([](const Dag::Operation& _op) {return _op.primitive() == Dag::Operation::EPrimitive::Refine; }) };
			if (refineIt != child.children().end())
			{
				unapplyRefineRecursive(_mesher, static_cast<Dag::Refine&>(*refineIt), false);
			}
		}
		unapplyRefine(_mesher, _refine, _detach);
	}

	Dag::Delete& prepareDelete()
	{
		return *new Dag::Delete{};
	}

	void applyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete)
	{
		Dag::Element& element{ _delete.parents().single() };
		_mesher.remove(element);
	}

	void unapplyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete, bool _detach)
	{
		_mesher.add(_delete.parents().single());
		if (_detach)
		{
			_delete.parents().detachAll(false);
		}
	}

	Dag::Extrude& prepareExtrude(const std::array<Id, 3>& _faceOffsets, Dag::Extrude::ESource _source)
	{
		Dag::Extrude& extrude{ *new Dag::Extrude{} };
		extrude.faceOffsets() = _faceOffsets;
		extrude.source() = _source;
		extrude.children().attach(*new Dag::Element{});
		return extrude;
	}

	void applyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _extrude)
	{
		Dag::Element& element{ _extrude.parents().single() };
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ _mesher.elementToPid(element) };
		const Id forwardFid{ mesh.poly_face_id(pid, _extrude.forwardFaceOffset()) };
		const Id upFid{ mesh.poly_face_id(pid, _extrude.upFaceOffset()) };
		const Id upEid{ mesh.face_shared_edge(forwardFid, upFid) };
		const FaceVertIds faceVids{ Meshing::Utils::faceVids(mesh, pid, forwardFid, upEid, true) };
		const FaceVerts faceVerts{ Meshing::Utils::verts(mesh, faceVids) };
		Real avgFaceEdgeLength{};
		{
			const std::vector<Id> faceEids{ mesh.adj_f2e(forwardFid) };
			for (const Id eid : faceEids)
			{
				avgFaceEdgeLength += mesh.edge_length(eid);
			}
			avgFaceEdgeLength /= faceEids.size();
		}
		PolyVerts& verts{ _extrude.children().single().vertices() };
		std::copy(faceVerts.begin(), faceVerts.end(), verts.begin());
		const Vec faceNormal = mesh.poly_face_normal(pid, forwardFid);
		int i{ 4 };
		for (const Vec& faceVert : faceVerts)
		{
			verts[i++] = faceVert + faceNormal * avgFaceEdgeLength;
		}
		_mesher.add(_extrude.children().single());
	}

	void unapplyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _extrude, bool _detach)
	{
		if (_detach)
		{
			_extrude.parents().detachAll(false);
		}
		_mesher.remove(_extrude.children().single());
	}

	void applyTree(Meshing::Mesher& _mesher, Dag::Node& _node)
	{
		for (Dag::Node* node : Dag::Utils::descendants(_node))
		{
			if (node->isOperation())
			{
				switch (node->operation().primitive())
				{
					case Dag::Operation::EPrimitive::Delete:
						applyDelete(_mesher, static_cast<Dag::Delete&>(*node));
						break;
					case Dag::Operation::EPrimitive::Extrude:
						applyExtrude(_mesher, static_cast<Dag::Extrude&>(*node));
						break;
					case Dag::Operation::EPrimitive::Refine:
						applyRefine(_mesher, static_cast<Dag::Refine&>(*node));
						break;
					default:
						throw std::domain_error{ "unexpected primitive" };
				}
			}
		}
	}


	void Sub3x3AdapterCandidate::setup3x3Subdivide(const Meshing::Mesher& _mesher)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ _mesher.elementToPid(*m_element) };
		m_scheme = Meshing::ERefinementScheme::Subdivide3x3;
		m_forwardFaceOffset = 0;
		const Id forwardFid{ mesh.poly_face_id(pid, m_forwardFaceOffset) };
		const Id upFid{ Meshing::Utils::adjacentFid(mesh, pid, forwardFid, mesh.face_edge_id(forwardFid, 0)) };
		m_upFaceOffset = mesh.poly_face_offset(pid, upFid);
	}

	void Sub3x3AdapterCandidate::findRightAdapter(const Meshing::Mesher& _mesher)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ _mesher.elementToPid(*m_element) };
		// process adjacent faces ignoring adjacent edges first
		switch (m_adjacentFaceOffsets.size())
		{
			case 0:
			{
				// no adjacent faces, skipping (the adapter will be chosen later, when considering edge adjacencies)
				m_scheme = std::nullopt;
			}
			break;
			case 1:
			{
				// single adjacent face -> AdapterFaceSubdivide3x3
				m_scheme = Meshing::ERefinementScheme::AdapterFaceSubdivide3x3;
				m_forwardFaceOffset = m_adjacentFaceOffsets[0];
				const Id forwardFid{ mesh.poly_face_id(pid, m_forwardFaceOffset) };
				const Id upFid{ Meshing::Utils::adjacentFid(mesh, pid, forwardFid, mesh.face_edge_id(forwardFid, 0)) };
				m_upFaceOffset = mesh.poly_face_offset(pid, upFid);
			}
			break;
			case 2:
			{
				// two adjacent faces
				const Id fid1{ mesh.poly_face_id(pid, m_adjacentFaceOffsets[0]) };
				const Id fid2{ mesh.poly_face_id(pid, m_adjacentFaceOffsets[1]) };
				if (mesh.faces_are_adjacent(fid1, fid2))
				{
					// if the two faces are adjacent -> Adapter2FacesSubdivide3x3
					m_scheme = Meshing::ERefinementScheme::Adapter2FacesSubdivide3x3;
					m_forwardFaceOffset = m_adjacentFaceOffsets[0];
					m_upFaceOffset = m_adjacentFaceOffsets[1];
				}
				else
				{
					// otherwise -> Subdivide3x3
					setup3x3Subdivide(_mesher);
				}
			}
			break;
			case 3:
			case 4:
			case 5:
			case 6:
			{
				// 3 or more adjacent faces -> Subdivide3x3
				setup3x3Subdivide(_mesher);
				break;
			}
			default:
				throw std::logic_error{ "unexpected number of adjacent faces" };
		}
		// now consider adjacent edges too
		// if the scheme is Subdivide3x3, leave it as it is (no edge adjacency can change this)
		if (!m_adjacentEdgeVertOffsets.empty() && m_scheme != Meshing::ERefinementScheme::Subdivide3x3)
		{
			// collect unprocessed edges (edges that are not part of an adjacent face)
			std::vector<Id> unprocessedEids{};
			for (const EdgeVertIds& adjEdgeVertOffs : m_adjacentEdgeVertOffsets)
			{
				const EdgeVertIds vids{ Meshing::Utils::edgeVids(mesh, adjEdgeVertOffs, pid) };
				const Id eid{ static_cast<Id>(mesh.edge_id(vids[0], vids[1])) };
				bool processed{ false };
				for (const Id faceOffset : m_adjacentFaceOffsets)
				{
					const Id fid{ mesh.poly_face_id(pid, faceOffset) };
					if (mesh.face_contains_edge(fid, eid))
					{
						processed = true;
						break;
					}
				}
				if (!processed)
				{
					unprocessedEids.push_back(eid);
				}
			}
			if (!unprocessedEids.empty())
			{
				if (unprocessedEids.size() == 1 && !m_scheme)
				{
					// no adapter has already been applied and there is only a single unprocessed edge -> AdapterEdgeSubdivide3x3
					m_scheme = Meshing::ERefinementScheme::AdapterEdgeSubdivide3x3;
					const Id targetForwardFid{ Meshing::Utils::anyFid(mesh, pid, unprocessedEids[0]) };
					m_forwardFaceOffset = mesh.poly_face_offset(pid, targetForwardFid);
					const Id targetUpFid{ Meshing::Utils::adjacentFid(mesh, pid, targetForwardFid, unprocessedEids[0]) };
					m_upFaceOffset = mesh.poly_face_offset(pid, targetUpFid);
				}
				else
				{
					// otherwise the only choice is to do Subdivide3x3
					setup3x3Subdivide(_mesher);
				}
			}
		}
	}

	Sub3x3AdapterCandidate::Sub3x3AdapterCandidate(Dag::Element& _element): m_element{ &_element }, m_scheme{} {}

	Dag::Element& Sub3x3AdapterCandidate::element() const
	{
		return *m_element;
	}

	Meshing::ERefinementScheme Sub3x3AdapterCandidate::scheme() const
	{
		return *m_scheme;
	}

	void Sub3x3AdapterCandidate::addAdjacency(const Meshing::Mesher& _mesher, const Dag::Element& _refined, bool _edge)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ _mesher.elementToPid(*m_element) };
		const Id refPid{ _mesher.elementToPid(_refined) };
		const Id sharedFid{ static_cast<Id>(mesh.poly_shared_face(pid, refPid)) };
		if (_edge)
		{
			if (sharedFid == noId) // skip if already processed as a face adjacency
			{
				// add the shared edge to the adjacency list
				const Id sharedEid{ Meshing::Utils::sharedEid(mesh, pid, refPid) };
				const EdgeVertIds edgeVertOffsets{ Meshing::Utils::edgePolyVertOffsets(mesh, sharedEid, pid) };
				m_adjacentEdgeVertOffsets.push_back(edgeVertOffsets);
			}
		}
		else
		{
			// add the shared face to the adjacency list
			m_adjacentFaceOffsets.push_back(mesh.poly_face_offset(pid, sharedFid));
		}
		findRightAdapter(_mesher);
	}

	Dag::Refine& Sub3x3AdapterCandidate::prepareAdapter(const Meshing::Mesher& _mesher) const
	{
		return Utils::prepareRefine(m_forwardFaceOffset, m_upFaceOffset, *m_scheme);
	}

	void Sub3x3AdapterCandidateSet::addAdjacency(const Meshing::Mesher& _mesher, Dag::Element& _candidate, const Dag::Element& _refined, bool _edge)
	{
		// find the map that contains the candidate (if any) and the iterator
		Map* map{ &m_sub3x3Map };
		auto it{ m_sub3x3Map.find(&_candidate) };
		if (it == m_sub3x3Map.end())
		{
			it = m_nonSub3x3Map.find(&_candidate);
			map = &m_nonSub3x3Map;
		}
		// get the existing candidate or create a new one
		Sub3x3AdapterCandidate candidate{ it == map->end() ? Sub3x3AdapterCandidate{_candidate} : it->second };
		candidate.addAdjacency(_mesher, _refined, _edge);
		// remove it from the previous map (if any) and add it again since it changed
		if (it != map->end())
		{
			map->erase(it);
		}
		Map& newMap{ (candidate.scheme() == Meshing::ERefinementScheme::Subdivide3x3) ? m_sub3x3Map : m_nonSub3x3Map };
		newMap.insert({ &_candidate, candidate });
	}

	void Sub3x3AdapterCandidateSet::addAdjacency(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		Dag::Element& refEl = _refine.parents().single();
		// temporarily add the element just to examine the adjacencies
		_mesher.add(refEl);
		const Id refPid{ _mesher.elementToPid(refEl) };
		// add face to face adjacent elements
		for (const Id candPid : mesh.adj_p2p(refPid))
		{
			addAdjacency(_mesher, _mesher.pidToElement(candPid), refEl, false);
		}
		// add face to edge adjacent elements
		for (const Id sharedEid : mesh.adj_p2e(refPid)) // for each adjacent edge sharedEid
		{
			for (const Id candPid : mesh.adj_e2p(sharedEid)) // for each adjacent element candPid to sharedEid
			{
				// if candPid is not the refined element, nor is adjacent face to face to it
				if (candPid != refPid && mesh.poly_shared_face(candPid, refPid) == noId)
				{
					addAdjacency(_mesher, _mesher.pidToElement(candPid), refEl, true);
				}
			}
		}
		// remove the temporarily added element
		_mesher.remove(refEl);
	}

	Sub3x3AdapterCandidate Sub3x3AdapterCandidateSet::pop()
	{
		// get the Subdivide3x3 candidates first
		Map& map{ m_sub3x3Map.empty() ? m_nonSub3x3Map : m_sub3x3Map };
		const auto it{ map.begin() };
		const Sub3x3AdapterCandidate candidate{ it->second };
		map.erase(it);
		return candidate;
	}

	bool Sub3x3AdapterCandidateSet::empty() const
	{
		return m_sub3x3Map.empty() && m_nonSub3x3Map.empty();
	}

}