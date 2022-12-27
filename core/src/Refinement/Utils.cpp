#include <HMP/Refinement/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <cassert>
#include <vector>
#include <cstddef>
#include <algorithm>

namespace HMP::Refinement::Utils
{

	Dag::Refine& prepare(I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth)
	{
		assert(_depth >= 1 && _depth <= 3);
		Dag::Refine& refine{ *new Dag::Refine{} };
		refine.scheme = _scheme;
		refine.forwardFi = _forwardFi;
		refine.firstVi = _firstVi;
		const Refinement::Scheme& refinement{ Refinement::schemes.at(_scheme) };
		for (I i{ 0 }; i < refinement.polys().size(); i++)
		{
			Dag::Element& child{ *new Dag::Element{} };
			if (_depth > 1)
			{
				child.children.attach(prepare(_forwardFi, _firstVi, _scheme, _depth - 1));
			}
			refine.children.attach(child);
		}
		return refine;
	}

	void apply(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const Dag::Element& element{ _refine.parents.single() };
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Refinement::Scheme& refinement{ Refinement::schemes.at(_refine.scheme) };
		const Id forwardFid{ Meshing::Utils::fid(mesh, element, _refine.forwardFi) };
		const Id firstVid{ element.vids[_refine.firstVi] };
		const HexVertIds sourceVids{ Meshing::Utils::pidVidsByForwardFidAndFirstVid(mesh, element.pid, forwardFid, firstVid) };
		const HexVerts sourceVerts{ Meshing::Utils::verts(mesh, sourceVids) };
		std::vector<Vec> newVerts;
		const std::vector<HexVertIds> polys{ refinement.apply(_mesher, sourceVerts, newVerts) };
		for (const auto& [child, vids] : cpputils::range::zip(_refine.children, polys))
		{
			child.vids = vids;
		}
		_mesher.add(_refine.children.address().toVector(), newVerts);
		_mesher.show(_refine.parents.single(), false);
	}

	void applyRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		apply(_mesher, _refine);
		for (Dag::Element& child : _refine.children)
		{
			for (Dag::Operation& operation : child.children)
			{
				if (operation.primitive == Dag::Operation::EPrimitive::Refine)
				{
					applyRecursive(_mesher, static_cast<Dag::Refine&>(operation));
				}
			}
		}
	}

	void Sub3x3AdapterCandidate::setup3x3Subdivide(const Meshing::Mesher& _mesher)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ m_element->pid };
		m_scheme = Refinement::EScheme::Subdivide3x3;
		m_forwardFaceOffset = 0;
		const Id forwardFid{ mesh.poly_face_id(pid, m_forwardFaceOffset) };
		const Id upFid{ Meshing::Utils::adjFidInPidByFidAndEid(mesh, pid, forwardFid, mesh.face_edge_id(forwardFid, 0)) };
		m_upFaceOffset = mesh.poly_face_offset(pid, upFid);
	}

	void Sub3x3AdapterCandidate::findRightAdapter(const Meshing::Mesher& _mesher)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ m_element->pid };
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
				m_scheme = Refinement::EScheme::AdapterFaceSubdivide3x3;
				m_forwardFaceOffset = m_adjacentFaceOffsets[0];
				const Id forwardFid{ mesh.poly_face_id(pid, m_forwardFaceOffset) };
				const Id upFid{ Meshing::Utils::adjFidInPidByFidAndEid(mesh, pid, forwardFid, mesh.face_edge_id(forwardFid, 0)) };
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
					m_scheme = Refinement::EScheme::Adapter2FacesSubdivide3x3;
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
				assert(false);
		}
		// now consider adjacent edges too
		// if the scheme is Subdivide3x3, leave it as it is (no edge adjacency can change this)
		if (!m_adjacentEdgeVertOffsets.empty() && m_scheme != Refinement::EScheme::Subdivide3x3)
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
					m_scheme = Refinement::EScheme::AdapterEdgeSubdivide3x3;
					const Id targetForwardFid{ Meshing::Utils::anyAdjFidInPidByEid(mesh, pid, unprocessedEids[0]) };
					m_forwardFaceOffset = mesh.poly_face_offset(pid, targetForwardFid);
					const Id targetUpFid{ Meshing::Utils::adjFidInPidByFidAndEid(mesh, pid, targetForwardFid, unprocessedEids[0]) };
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

	Refinement::EScheme Sub3x3AdapterCandidate::scheme() const
	{
		return *m_scheme;
	}

	void Sub3x3AdapterCandidate::addAdjacency(const Meshing::Mesher& _mesher, const Dag::Element& _refined, bool _edge)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ m_element->pid };
		const Id refPid{ _refined.pid };
		const Id sharedFid{ static_cast<Id>(mesh.poly_shared_face(pid, refPid)) };
		if (_edge)
		{
			if (sharedFid == noId) // skip if already processed as a face adjacency
			{
				// add the shared edge to the adjacency list
				const Id sharedEid{ Meshing::Utils::sharedEid(mesh, pid, refPid) };
				const EdgeVertIds edgeVertOffsets{ Meshing::Utils::edgeHexVertOffsets(mesh, sharedEid, pid) };
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
		return Utils::prepare(m_forwardFaceOffset, m_upFaceOffset, *m_scheme);
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
		Map& newMap{ (candidate.scheme() == Refinement::EScheme::Subdivide3x3) ? m_sub3x3Map : m_nonSub3x3Map };
		newMap.insert({ &_candidate, candidate });
	}

	void Sub3x3AdapterCandidateSet::addAdjacency(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		Dag::Element& refEl = _refine.parents.single();
		const Id refPid{ refEl.pid };
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