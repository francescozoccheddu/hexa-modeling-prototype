#include <HMP/Actions/MakeConforming.hpp>

#include <HMP/Meshing/refinementSchemes.hpp>
#include <HMP/Dag/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	using Meshing::ERefinementScheme;

	MakeConforming::RefinementMap MakeConforming::findStandardRefinements()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		std::unordered_map<ERefinementScheme, std::vector<Dag::Refine*>> standardRefines{};
		// find standard refines subdivide3x3 and inset
		{
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
		}
		return standardRefines;
	}

	std::vector<Dag::Refine*> MakeConforming::installInset(const std::vector<Dag::Refine*>& _insets)
	{
		std::vector<Dag::Refine*> installed{};
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		for (Dag::Refine* standardRefine : _insets)
		{
			Dag::Refine& sourceRefine = *standardRefine;
			Dag::Element& sourceElement = sourceRefine.parents().single();
			mesher.add(sourceElement);
			const Id sourcePid{ mesher.elementToPid(sourceElement) };
			for (Id targetPid : mesh.adj_p2p(sourcePid))
			{
				if (targetPid == sourcePid)
				{
					continue;
				}
				const Id sharedFid = mesh.poly_shared_face(sourcePid, targetPid);
				if (sharedFid == noId)
				{
					continue;
				}
				if (sharedFid != mesh.poly_face_id(sourcePid, sourceRefine.forwardFaceOffset()))
				{
					continue;
				}
				Dag::Element& targetElement = mesher.pidToElement(targetPid);
				const Id targetForwardFid{ sharedFid };
				const Id targetForwardFaceOffset{ mesh.poly_face_offset(targetPid, targetForwardFid) };
				const Id targetUpFid{ Meshing::Utils::adjacentFid(mesh, targetPid, targetForwardFid, mesh.face_edge_id(targetForwardFid, 0)) };
				const Id targetUpFaceOffset{ mesh.poly_face_offset(targetPid, targetUpFid) };
				Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, ERefinementScheme::Inset) };
				targetRefine.parents().attach(targetElement);
				Utils::applyRefine(mesher, targetRefine);
				m_operations.push_back({ &targetRefine, &targetElement });
				installed.push_back(&targetRefine);
			}
			mesher.remove(sourceElement);
		}
		return installed;
	}

	std::vector<Dag::Refine*> MakeConforming::installAdapterEdgeSubdivide3x3(const std::vector<Dag::Refine*>& _sub3x3s)
	{
		std::vector<Dag::Refine*> installed{};
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		for (Dag::Refine* standardRefine : _sub3x3s)
		{
			Dag::Refine& sourceRefine = *standardRefine;
			Dag::Element& sourceElement = sourceRefine.parents().single();
			mesher.add(sourceElement);
			const Id sourcePid{ mesher.elementToPid(sourceElement) };
			for (Id sharedEid : mesh.adj_p2e(sourcePid))
			{
				for (Id targetPid : mesh.adj_e2p(sharedEid))
				{
					if (targetPid == sourcePid || mesh.poly_shared_face(targetPid, sourcePid) != noId)
					{
						continue;
					}
					Dag::Element& targetElement = mesher.pidToElement(targetPid);
					const Id targetForwardFid{ Meshing::Utils::anyFid(mesh, targetPid, sharedEid) };
					const Id targetForwardFaceOffset{ mesh.poly_face_offset(targetPid, targetForwardFid) };
					const Id targetUpFid{ Meshing::Utils::adjacentFid(mesh, targetPid, targetForwardFid, sharedEid) };
					const Id targetUpFaceOffset{ mesh.poly_face_offset(targetPid, targetUpFid) };
					Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, ERefinementScheme::AdapterEdgeSubdivide3x3) };
					targetRefine.parents().attach(targetElement);
					Utils::applyRefine(mesher, targetRefine);
					m_operations.push_back({ &targetRefine, &targetElement });
					installed.push_back(&targetRefine);
				}
			}
			mesher.remove(sourceElement);
		}
		return installed;
	}

	std::vector<Dag::Refine*> MakeConforming::installAdapterFaceSubdivide3x3(const std::vector<Dag::Refine*>& _sub3x3s)
	{
		std::vector<Dag::Refine*> installed{};
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		for (Dag::Refine* standardRefine : _sub3x3s)
		{
			Dag::Refine& sourceRefine = *standardRefine;
			Dag::Element& sourceElement = sourceRefine.parents().single();
			mesher.add(sourceElement);
			const Id sourcePid{ mesher.elementToPid(sourceElement) };
			for (Id targetPid : mesh.adj_p2p(sourcePid))
			{
				if (targetPid == sourcePid)
				{
					continue;
				}
				const Id sharedFid = mesh.poly_shared_face(sourcePid, targetPid);
				if (sharedFid == noId)
				{
					continue;
				}
				Dag::Element& targetElement = mesher.pidToElement(targetPid);
				const Id targetForwardFid{ sharedFid };
				const Id targetForwardFaceOffset{ mesh.poly_face_offset(targetPid, targetForwardFid) };
				const Id targetUpFid{ Meshing::Utils::adjacentFid(mesh, targetPid, targetForwardFid, mesh.face_edge_id(targetForwardFid, 0)) };
				const Id targetUpFaceOffset{ mesh.poly_face_offset(targetPid, targetUpFid) };
				Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, ERefinementScheme::AdapterFaceSubdivide3x3) };
				targetRefine.parents().attach(targetElement);
				Utils::applyRefine(mesher, targetRefine);
				m_operations.push_back({ &targetRefine, &targetElement });
				installed.push_back(&targetRefine);
			}
			mesher.remove(sourceElement);
		}
		return installed;
	}

	std::vector<Dag::Refine*> MakeConforming::installSubdivide3x3(const std::vector<Dag::Refine*>& _sub3x3s)
	{
		std::vector<Dag::Refine*> installed{};
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		std::unordered_set<Dag::Element*> targetCandidates;
		for (Dag::Refine* standardRefine : _sub3x3s)
		{
			Dag::Refine& sourceRefine = *standardRefine;
			Dag::Element& sourceElement = sourceRefine.parents().single();
			mesher.add(sourceElement);
			const Id sourcePid{ mesher.elementToPid(sourceElement) };
			for (const Id targetPid : mesh.adj_p2p(sourcePid))
			{
				Dag::Element& targetElement = mesher.pidToElement(targetPid);
				if (!targetCandidates.insert(&targetElement).second)
				{
					const Id targetForwardFaceOffset{ 0 };
					const Id targetForwardFid{ mesh.poly_face_id(targetPid, targetForwardFaceOffset) };
					const Id targetUpFid{ Meshing::Utils::adjacentFid(mesh, targetPid, targetForwardFid, mesh.face_edge_id(targetForwardFid, 0)) };
					const Id targetUpFaceOffset{ mesh.poly_face_offset(targetPid, targetUpFid) };
					Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, ERefinementScheme::Subdivide3x3) };
					targetRefine.parents().attach(targetElement);
					Utils::applyRefine(mesher, targetRefine);
					m_operations.push_back({ &targetRefine, &targetElement });
					installed.push_back(&targetRefine);
				}
			}
			mesher.remove(sourceElement);
		}
		return installed;
	}

	std::vector<Dag::Refine*> MakeConforming::installAdapter2FacesSubdivide3x3(const std::vector<Dag::Refine*>& _sub3x3s)
	{
		std::vector<Dag::Refine*> installed{};
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		std::unordered_set<Dag::Element*> targetCandidates;
		return installed;
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
			const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
			bool didSomething;
			do
			{
				didSomething = false;
				RefinementMap refinesMap{ findStandardRefinements() };
				std::vector<Dag::Refine*>& sub3x3s{ refinesMap.at(ERefinementScheme::Subdivide3x3) };
				std::vector<Dag::Refine*>& insets{ refinesMap.at(ERefinementScheme::Inset) };
				{
					bool didSomeSub3x3{ false };
					do
					{
						bool didSomeAd2F3x3{ false };
						do
						{
							const std::vector<Dag::Refine*> installed{ installAdapter2FacesSubdivide3x3(sub3x3s) };
							didSomeAd2F3x3 = !installed.empty();
						}
						while (didSomeAd2F3x3);
						const std::vector<Dag::Refine*> installed{ installSubdivide3x3(sub3x3s) };
						didSomeSub3x3 = !installed.empty();
						sub3x3s.insert(sub3x3s.end(), installed.begin(), installed.end());
					}
					while (didSomeSub3x3);
					installAdapterFaceSubdivide3x3(sub3x3s);
					installAdapterEdgeSubdivide3x3(sub3x3s);
					installInset(insets);
				}
			}
			while (didSomething);
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