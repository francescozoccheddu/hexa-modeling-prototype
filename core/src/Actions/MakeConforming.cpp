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
		const std::vector<Dag::Refine*> refines{ _insets };
		for (Dag::Refine* const refine : refines)
		{
			Dag::Refine& ref = *refine;
			Dag::Element& refEl = ref.parents().single();
			// temporarily add the element just to examine the adjacencies
			mesher.add(refEl);
			const Id refPid{ mesher.elementToPid(refEl) };
			// for each adjacent poly candPid
			for (const Id candPid : mesh.adj_p2p(refPid))
			{
				// skip if self-adjacent
				if (candPid == refPid)
				{
					continue;
				}
				const Id sharedFid = mesh.poly_shared_face(refPid, candPid);
				// skip if they do not share a face
				if (sharedFid == noId)
				{
					continue;
				}
				// skip if the shared face is not the refined one
				if (sharedFid != mesh.poly_face_id(refPid, ref.forwardFaceOffset()))
				{
					continue;
				}
				// get the orientation right
				Dag::Element& candEl = mesher.pidToElement(candPid);
				const Id candForwardFid{ sharedFid };
				const Id candForwardFaceOffset{ mesh.poly_face_offset(candPid, candForwardFid) };
				const Id candUpFid{ Meshing::Utils::adjacentFid(mesh, candPid, candForwardFid, mesh.face_edge_id(candForwardFid, 0)) };
				const Id candUpFaceOffset{ mesh.poly_face_offset(candPid, candUpFid) };
				// apply the refinement
				Dag::Refine& adapterRef{ Utils::prepareRefine(candForwardFaceOffset, candUpFaceOffset, ERefinementScheme::Inset) };
				adapterRef.parents().attach(candEl);
				Utils::applyRefine(mesher, adapterRef);
				m_operations.push_back({ &adapterRef, &candEl });
				_insets.push_back(&adapterRef);
			}
			// remove the temporarily added element
			mesher.remove(refEl);
		}
	}

	void MakeConforming::installSubdivide3x3Adapters(std::vector<Dag::Refine*>& _sub3x3s)
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		bool didSomething{ false };
		do
		{
			Utils::Sub3x3AdapterCandidateSet set{};
			// build a set of candidates based on the source refinements
			for (Dag::Refine* refine : _sub3x3s)
			{
				set.addAdjacency(mesher, *refine);
			}
			didSomething = !set.empty();
			// while there is a candidate
			while (!set.empty())
			{
				// prepare and apply its refinement
				const Utils::Sub3x3AdapterCandidate candidate{ set.pop() };
				Dag::Refine& adapterRefine{ candidate.prepareAdapter(mesher) };
				adapterRefine.parents().attach(candidate.element());
				Utils::applyRefine(mesher, adapterRefine);
				m_operations.push_back({ &adapterRefine, &candidate.element() });
				// if the refinement is a new Subdivide3x3, then add it to the set
				if (candidate.scheme() == ERefinementScheme::Subdivide3x3)
				{
					set.addAdjacency(mesher, adapterRefine);
					_sub3x3s.push_back(&adapterRefine);
				}
			}
		}
		while (didSomething);
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