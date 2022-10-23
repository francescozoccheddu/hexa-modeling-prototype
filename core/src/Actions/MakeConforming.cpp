#include <HMP/Actions/MakeConforming.hpp>

#include <HMP/Meshing/refinementSchemes.hpp>
#include <HMP/Dag/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <optional>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	using Meshing::ERefinementScheme;

	MakeConforming::RefinementMap MakeConforming::findStandardRefinements(const std::set<Meshing::ERefinementScheme>& _schemes)
	{
		Meshing::Mesher& mesher{ this->mesher() };
		std::unordered_map<ERefinementScheme, std::vector<Dag::Refine*>> standardRefines{};
		// find standard subdivide3x3 and inset refinements
		constexpr static std::array schemesToFind{
			ERefinementScheme::Subdivide3x3,
			ERefinementScheme::Inset,
		};
		// map scheme -> refinements
		standardRefines.reserve(schemesToFind.size());
		for (ERefinementScheme scheme : schemesToFind)
		{
			standardRefines.insert({ scheme, {} });
		}
		for (Dag::Node* node : Dag::Utils::descendants(*root()))
		{
			if (node->isOperation() && node->operation().primitive() == Dag::Operation::EPrimitive::Refine)
			{
				Dag::Refine& refine{ static_cast<Dag::Refine&>(node->operation()) };
				const auto mapIt{ standardRefines.find(refine.scheme()) };
				if (mapIt != standardRefines.end())
				{
					// check if no move operations were applied
					if (c_requireNoMove)
					{
						// get parent vids
						std::unordered_set<Id> vids{};
						{
							// temporarily add the parent, just to get its vertices
							mesher.add(refine.parents().single());
							// get its hypothetical vertices
							const std::vector<PolyVerts> polys{ Utils::previewRefine(mesher, refine) };
							// remove the temporarily added parent element
							mesher.remove(refine.parents().single());
							// map vertices to vids
							for (const PolyVerts verts : polys)
							{
								for (const Vec& vert : verts)
								{
									const Id vid{ mesher.getVert(vert) };
									// this vertex is no more in the mesh, so a move operation happened
									if (vid == noId)
									{
										goto nextRefine;
									}
									vids.insert(vid);
								}
							}
						}
						// check if children vertices have not moved with respect to the original refine
						for (const Dag::Element& child : refine.children())
						{
							const Id childPid{ mesher.elementToPid(child) };
							// if child is still in the mesh, check its vids
							if (childPid != noId)
							{
								for (const Id vid : mesher.mesh().poly_verts_id(mesher.elementToPid(child)))
								{
									if (!vids.contains(vid))
									{
										goto nextRefine;
									}
								}
							}
							// otherwise, check its vertex positions
							else
							{
								for (const Vec& vert : child.vertices())
								{
									if (!vids.contains(mesher.getVert(vert)))
									{
										goto nextRefine;
									}
								}
							}
						}
					}
					// check if no delete operations were applied
					if (c_requireNoDelete)
					{
						for (const Dag::Element& child : refine.children())
						{
							if (child.children().any([](const Dag::Operation& _childOperation) { return _childOperation.primitive() == Dag::Operation::EPrimitive::Delete; }))
							{
								goto nextRefine;
							}
						}
					}
					// add to the list of standard refinements
					mapIt->second.push_back(&refine);
				}
			}
		nextRefine:;
		}
		return standardRefines;
	}

	void MakeConforming::installInsetAdapters(std::vector<Dag::Refine*>& _insets)
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		// copy the source refinements because we don't want to consider the newly added refines
		const std::vector<Dag::Refine*> standardRefines{ _insets };
		for (Dag::Refine* standardRefine : standardRefines)
		{
			Dag::Refine& sourceRefine = *standardRefine;
			Dag::Element& sourceElement = sourceRefine.parents().single();
			// temporarily add the element just to examine the adjacencies
			mesher.add(sourceElement);
			const Id sourcePid{ mesher.elementToPid(sourceElement) };
			// for each adjacent poly targetPid
			for (Id targetPid : mesh.adj_p2p(sourcePid))
			{
				// skip if self-adjacent
				if (targetPid == sourcePid)
				{
					continue;
				}
				const Id sharedFid = mesh.poly_shared_face(sourcePid, targetPid);
				// skip if they do not share a face
				if (sharedFid == noId)
				{
					continue;
				}
				// skip if the shared face is not the refined one
				if (sharedFid != mesh.poly_face_id(sourcePid, sourceRefine.forwardFaceOffset()))
				{
					continue;
				}
				// get the orientation right
				Dag::Element& targetElement = mesher.pidToElement(targetPid);
				const Id targetForwardFid{ sharedFid };
				const Id targetForwardFaceOffset{ mesh.poly_face_offset(targetPid, targetForwardFid) };
				const Id targetUpFid{ Meshing::Utils::adjacentFid(mesh, targetPid, targetForwardFid, mesh.face_edge_id(targetForwardFid, 0)) };
				const Id targetUpFaceOffset{ mesh.poly_face_offset(targetPid, targetUpFid) };
				// apply the refinement
				Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, ERefinementScheme::Inset) };
				targetRefine.parents().attach(targetElement);
				Utils::applyRefine(mesher, targetRefine);
				m_operations.push_back({ &targetRefine, &targetElement });
				_insets.push_back(&targetRefine);
			}
			// remove the temporarily added element
			mesher.remove(sourceElement);
		}
	}

	class Sub3x3AdapterCandidate final
	{

	private:

		Dag::Element* m_element;
		std::optional<ERefinementScheme> m_scheme;
		Id m_forwardFaceOffset, m_upFaceOffset;

	public:

		Sub3x3AdapterCandidate(Dag::Element& _element) : m_element{ &_element }, m_scheme{} {}

		Dag::Element& element() const
		{
			return *m_element;
		}

		ERefinementScheme scheme() const
		{
			return *m_scheme;
		}

		void addAdjacency(const Meshing::Mesher& _mesher, const Dag::Element& _refined, bool _edge)
		{
			throw std::logic_error{ "not implemented yet" }; // TODO
		}

		Dag::Refine& prepareAdapter(const Meshing::Mesher& _mesher) const
		{
			return Utils::prepareRefine(m_forwardFaceOffset, m_upFaceOffset, *m_scheme);
		}

	};

	class Sub3x3AdapterCandidateSet final
	{

	private:

		using Map = std::unordered_map<Dag::Element*, Sub3x3AdapterCandidate>;

		Map m_sub3x3Map{}; // candidates that will be refined as a new Subdivide3x3 (they need to be processed first)
		Map m_nonSub3x3Map{}; // other candidates (they must be processed only after)

		void addAdjacency(const Meshing::Mesher& _mesher, Dag::Element& _candidate, const Dag::Element& _refined, bool _edge)
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
			// if the candidate was not previously stored or it changed its target map
			if ((candidate.scheme() == ERefinementScheme::Subdivide3x3) != (map == &m_sub3x3Map))
			{
				// remove it from the previous map (if any) and add it to the right one
				if (it != map->end())
				{
					map->erase(it);
				}
				Map& newMap{ (candidate.scheme() == ERefinementScheme::Subdivide3x3) ? m_sub3x3Map : m_nonSub3x3Map };
				newMap.insert({ &_candidate, candidate });
			}
		}

	public:

		void addAdjacency(Meshing::Mesher& _mesher, Dag::Refine& _refine)
		{
			const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
			Dag::Element& sourceElement = _refine.parents().single();
			// temporarily add the element just to examine the adjacencies
			_mesher.add(sourceElement);
			const Id sourcePid{ _mesher.elementToPid(sourceElement) };
			// add face to face adjacent elements
			for (const Id targetPid : mesh.adj_p2p(sourcePid))
			{
				addAdjacency(_mesher, _mesher.pidToElement(targetPid), sourceElement, false);
			}
			// add face to edge adjacent elements
			for (const Id sharedEid : mesh.adj_p2e(sourcePid)) // for each adjacent edge sharedEid
			{
				for (const Id targetPid : mesh.adj_e2p(sharedEid)) // foreeach adjacent element targetPid to sharedEid
				{
					// if targetPid is not the source element, nor is adjacent face to face to it
					if (targetPid != sourcePid && mesh.poly_shared_face(targetPid, sourcePid) == noId)
					{
						addAdjacency(_mesher, _mesher.pidToElement(targetPid), sourceElement, true);
					}
				}
			}
			// remove the temporarily added element
			_mesher.remove(sourceElement);
		}

		const Sub3x3AdapterCandidate& pop()
		{
			Map& map{ m_sub3x3Map.empty() ? m_nonSub3x3Map : m_sub3x3Map };
			const auto it{ map.begin() };
			const Sub3x3AdapterCandidate candidate{ it->second };
			map.erase(it);
			return candidate;
		}

		bool empty() const
		{
			return m_sub3x3Map.empty() && m_nonSub3x3Map.empty();
		}

	};

	void MakeConforming::installSubdivide3x3Adapters(std::vector<Dag::Refine*>& _sub3x3s)
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		Sub3x3AdapterCandidateSet set{};
		// build a set of candidates based on the source refinements
		for (Dag::Refine* standardRefine : _sub3x3s)
		{
			set.addAdjacency(mesher, *standardRefine);
		}
		// while there is a candidate
		while (!set.empty())
		{
			// prepare and apply its refinement
			const Sub3x3AdapterCandidate candidate{ set.pop() };
			Dag::Refine& targetRefine{ candidate.prepareAdapter(mesher) };
			targetRefine.parents().attach(candidate.element());
			Utils::applyRefine(mesher, targetRefine);
			m_operations.push_back({ &targetRefine, &candidate.element() });
			// if the refinement is a new Subdivide3x3, then add it to the set
			if (candidate.scheme() == ERefinementScheme::Subdivide3x3)
			{
				set.addAdjacency(mesher, targetRefine);
				_sub3x3s.push_back(&targetRefine);
			}
		}
	}

	void MakeConforming::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		if (m_prepared)
		{
			for (auto [operation, element] : m_operations)
			{
				operation->parents().attach(*element);
				Utils::applyRefine(mesher, *operation);
			}
		}
		else
		{
			m_prepared = true;
			RefinementMap refinesMap{ findStandardRefinements({ERefinementScheme::Subdivide3x3, ERefinementScheme::Inset }) };
			installSubdivide3x3Adapters(refinesMap.at(ERefinementScheme::Subdivide3x3));
			installInsetAdapters(refinesMap.at(ERefinementScheme::Inset));
		}
		mesher.updateMesh();
	}

	void MakeConforming::unapply()
	{
		for (auto it{ m_operations.rbegin() }; it != m_operations.rend(); ++it)
		{
			auto& [operation, element] {*it};
			Utils::unapplyRefine(mesher(), *operation);
		}
		mesher().updateMesh();
	}

	MakeConforming::MakeConforming() : m_operations{}, m_prepared{ false }
	{}

	std::vector<std::pair<const Dag::Refine*, const Dag::Element*>> MakeConforming::operations() const
	{
		std::vector<std::pair<const Dag::Refine*, const Dag::Element*>> operations{};
		operations.reserve(m_operations.size());
		for (const std::pair<const Dag::Refine*, const Dag::Element*> operation : m_operations)
		{
			operations.push_back({ operation.first, operation.second });
		}
		return operations;
	}

}