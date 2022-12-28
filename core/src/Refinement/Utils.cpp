#include <HMP/Refinement/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <cinolib/geometry/lerp.hpp>
#include <cassert>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <unordered_set>
#include <limits>

namespace HMP::Refinement::Utils
{

	Dag::Refine& prepare(I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth)
	{
		assert(_depth >= 1 && _depth <= 3);
		Dag::Refine& refine{ *new Dag::Refine{} };
		refine.scheme = _scheme;
		refine.forwardFi = _forwardFi;
		refine.firstVi = _firstVi;
		const Refinement::Scheme& scheme{ Refinement::schemes.at(_scheme) };
		for (I i{ 0 }; i < scheme.polys.size(); i++)
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

	IVec schemeOrigin(I _vi, I _gridSize)
	{
		const I s{ _gridSize };
		switch (_vi)
		{
			case 0:
				return { 0,0,0 };
			case 1:
				return { s,0,0 };
			case 2:
				return { s,s,0 };
			case 3:
				return { 0,s,0 };
			case 4:
				return { 0,0,s };
			case 5:
				return { s,0,s };
			case 6:
				return { s,s,s };
			case 7:
				return { 0,s,s };
			default:
				assert(false);
		}
	}

	struct BasisVec final
	{
		bool dim;
		bool positive;

		constexpr BasisVec operator-() const
		{
			return { dim, !positive };
		}

	};

	I index(const QuadVertData<I>& _elements, I _element)
	{
		return static_cast<I>(std::distance(_elements.begin(), std::find(_elements.begin(), _elements.end(), _element)));
	}

	std::array<BasisVec, 2> rotateBasisCCW(I _count, const BasisVec& _right, const BasisVec& _up)
	{
		switch (_count % 4)
		{
			case 0:
				return { _right, _up };
			case 1:
				return { _up, -_right };
			case 2:
				return { -_right, -_up };
			case 3:
				return { -_up, _right };
			default:
				assert(false);
		}
	}

	static constexpr HexFaceData<QuadVertData<I>> faceVis{ {
		{ 0,1,2,3 },
		{ 4,7,6,5 },
		{ 1,5,6,2 },
		{ 0,3,7,4 },
		{ 0,4,5,1 },
		{ 3,2,6,7 }
	} };

	std::array<BasisVec, 2> schemeBasis(I _fi, I _vi)
	{
		static constexpr Id x{ 0 }, y{ 1 }, z{ 2 };
		static constexpr HexFaceData<std::pair<I, I>> dims{ {
			{x,y},
			{y,x},
			{z,y},
			{y,z},
			{z,x},
			{x,z}
		} };
		const auto& [rightDim, upDim] { dims[_fi] };
		return rotateBasisCCW(index(faceVis[_fi], _vi), { rightDim > upDim, true }, { upDim > rightDim, true });
	}

	I rotateFiCW(I _fi, I _forwardFi)
	{
		static constexpr HexFaceData<HexFaceData<I>> rotFis{ {
			{0,1,4,5,3,2},
			{0,1,5,4,2,3},
			{5,4,2,3,0,1},
			{4,5,2,3,1,0},
			{2,3,1,0,4,5},
			{3,2,0,1,4,5}
		} };
		return rotFis[_forwardFi][_fi];
	}

	I rotateFiCCW(I _fi, I _forwardFi)
	{
		static constexpr HexFaceData<HexFaceData<I>> rotFis{ {
			{0,1,5,4,2,3},
			{0,1,4,5,3,2},
			{4,5,2,3,1,0},
			{5,4,2,3,0,1},
			{3,2,0,1,4,5},
			{2,3,1,0,4,5}
		} };
		return rotFis[_forwardFi][_fi];
	}

	I relativeFi(I _fi, I _forwardFi, I _vi)
	{
		static constexpr HexFaceData<HexFaceData<I>> invFis{ {
			{0,1,2,3,4,5},
			{1,0,5,4,3,2},
			{3,2,0,1,4,5},
			{4,5,1,0,3,2},
			{3,2,5,4,0,1},
			{4,5,2,3,1,0}
		} };
		I count{ index(faceVis[_forwardFi], _vi) };
		while (count-- > 0)
		{
			_fi = rotateFiCW(_fi, _forwardFi);
		}
		return invFis[_forwardFi][_fi];
	}

	void apply(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const Scheme& scheme{ schemes.at(_refine.scheme) };
		std::vector<Id> schemeVids(scheme.verts.size(), noId);
		Dag::Element& parent{ _refine.parents.single() };
		const HexVertIds parentVids{ Meshing::Utils::align(Meshing::Utils::rotate(parent.vids, _refine.forwardFi), parent.vids[_refine.firstVi]) };
		// weld adjacencies
		{
			const IVec origin{ schemeOrigin(_refine.firstVi, scheme.gridSize) };
			const std::array<BasisVec, 2> basis{ schemeBasis(_refine.forwardFi, _refine.firstVi) };
			// ################# TEMP IMPL ##################
			std::unordered_set<Id> vidsSearchPool{}; // TEMP IMPL
			// ##############################################
			for (const Id adjPid : _mesher.mesh().adj_p2p(parent.pid))
			{
				const Dag::Refine* adjRefine{
					_mesher
					.element(adjPid)
					.children
					.filter([&](const Dag::Operation& _op) { return _op.primitive == Dag::Operation::EPrimitive::Refine; })
					.address()
					.cast<const Dag::Refine*>()
					.single(nullptr)
				};
				if (!adjRefine)
				{
					continue;
				}
				const Scheme& adjScheme{ schemes.at(adjRefine->scheme) };
				if (adjScheme.gridSize == scheme.gridSize)
				{
					const Id fid{ static_cast<Id>(_mesher.mesh().poly_shared_face(parent.pid, adjPid)) };
					const QuadVertIds fidVids{ Meshing::Utils::fidVids(_mesher.mesh(), fid) };
					const I fi{ Meshing::Utils::fi(parent.vids, fidVids) };
					const I adjFi{ Meshing::Utils::fi(adjRefine->parents.single().vids, fidVids) };
					const I schemeFi{ relativeFi(fi, _refine.forwardFi, _refine.firstVi) };
					const I adjSchemeFi{ relativeFi(adjFi, adjRefine->forwardFi, adjRefine->firstVi) };
					const I count{ scheme.facesSurfVisIs[schemeFi].size() / 4 };
					const I adjCount{ adjScheme.facesSurfVisIs[adjSchemeFi].size() / 4 };
					std::cout
						<< "fi=" << fi
						<< ", adjFi=" << adjFi
						<< ", forwardFi=" << _refine.forwardFi
						<< ", firstVi=" << _refine.firstVi
						<< ", adjForwardFi=" << adjRefine->forwardFi
						<< ", adjFirstVi=" << adjRefine->firstVi
						<< ", schemeFi=" << schemeFi
						<< ", adjSchemeFi=" << adjSchemeFi
						<< ", schemeFiSize=" << count
						<< ", adjSchemeFiSize=" << adjCount
						<< std::endl;
					// ################# TEMP IMPL ##################
					vidsSearchPool.insert(adjRefine->surfVids.begin(), adjRefine->surfVids.end()); // TEMP IMPL
					// ##############################################
				}
			}
			// ################# TEMP IMPL ##################
			{  // TEMP IMPL
				const Id numVerts{ _mesher.mesh().num_verts() }; // TEMP IMPL
				const HexVerts parentVerts{ Meshing::Utils::verts(_mesher.mesh(), parentVids) }; // TEMP IMPL
				const HexVerts lerpVerts{ // TEMP IMPL
					parentVerts[0], // TEMP IMPL
					parentVerts[1], // TEMP IMPL
					parentVerts[3], // TEMP IMPL
					parentVerts[2], // TEMP IMPL
					parentVerts[4], // TEMP IMPL
					parentVerts[5], // TEMP IMPL
					parentVerts[7], // TEMP IMPL
					parentVerts[6] // TEMP IMPL
				}; // TEMP IMPL
				const Real eps{ 1e-9 }; // TEMP IMPL
				for (const auto& [vid, ivert] : cpputils::range::zip(schemeVids, scheme.verts)) // TEMP IMPL
				{ // TEMP IMPL
					const Vec progress{ ivert.cast<Real>() / static_cast<Real>(scheme.gridSize) }; // TEMP IMPL
					const Vec vert{ cinolib::lerp3(lerpVerts, progress) }; // TEMP IMPL
					Real minDist{ std::numeric_limits<Real>::infinity() }; // TEMP IMPL
					for (const Id candVid : vidsSearchPool) // TEMP IMPL
					{ // TEMP IMPL
						const Real dist{ _mesher.mesh().vert(candVid).dist(vert) }; // TEMP IMPL
						if (dist < minDist && dist < eps) // TEMP IMPL
						{ // TEMP IMPL
							minDist = dist; // TEMP IMPL
							vid = candVid; // TEMP IMPL
						} // TEMP IMPL
					} // TEMP IMPL
				} // TEMP IMPL
			} // TEMP IMPL
			// ##############################################
		}
		// weld corners
		for (const I cornerVi : scheme.cornerVis)
		{
			const IVec& corner{ scheme.verts[cornerVi] };
			schemeVids[cornerVi] = parentVids[scheme.cornerVi(corner)];
		}
		// create missing verts
		std::vector<Vec> newVerts;
		{
			const Id numVerts{ _mesher.mesh().num_verts() };
			const HexVerts parentVerts{ Meshing::Utils::verts(_mesher.mesh(), parentVids) };
			const HexVerts lerpVerts{
				parentVerts[0],
				parentVerts[1],
				parentVerts[3],
				parentVerts[2],
				parentVerts[4],
				parentVerts[5],
				parentVerts[7],
				parentVerts[6]
			};
			for (const auto& [vid, ivert] : cpputils::range::zip(schemeVids, scheme.verts))
			{
				if (vid == noId)
				{
					vid = numVerts + toId(newVerts.size());
					const Vec progress{ ivert.cast<Real>() / static_cast<Real>(scheme.gridSize) };
					newVerts.push_back(cinolib::lerp3(lerpVerts, progress));
				}
			}
		}
		// create elements
		{
			const std::vector<Dag::Element*> elements{
				_refine.children.zip(scheme.polys).map([&](const auto& _elAndVis) {
					const auto& [el, vis] { _elAndVis };
					for (const auto& [vid, vi] : cpputils::range::zip(el.vids, vis))
					{
						vid = schemeVids[vi];
					}
					return &el;
			}).toVector() };
			_refine.surfVids = cpputils::range::of(scheme.surfVis).map([&](const I _vi) { return schemeVids[_vi]; }).toVector();
			_mesher.show(parent, false);
			_mesher.add(elements, newVerts);
		}
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

	Dag::Refine& Sub3x3AdapterCandidate::prepareAdapter() const
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
			addAdjacency(_mesher, _mesher.element(candPid), refEl, false);
		}
		// add face to edge adjacent elements
		for (const Id sharedEid : mesh.adj_p2e(refPid)) // for each adjacent edge sharedEid
		{
			for (const Id candPid : mesh.adj_e2p(sharedEid)) // for each adjacent element candPid to sharedEid
			{
				// if candPid is not the refined element, nor is adjacent face to face to it
				if (candPid != refPid && static_cast<Id>(mesh.poly_shared_face(candPid, refPid)) == noId)
				{
					addAdjacency(_mesher, _mesher.element(candPid), refEl, true);
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