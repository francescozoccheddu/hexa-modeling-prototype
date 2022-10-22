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
				std::unordered_map<Meshing::ERefinementScheme, std::vector<Dag::Refine*>> standardRefines{};
				standardRefines.reserve(Meshing::refinementSchemes.size());
				for (const auto& [scheme, refinement] : Meshing::refinementSchemes)
				{
					standardRefines.insert({ scheme, {} });
				}
				for (Dag::Node* node : Dag::Utils::descendants(*root()))
				{
					if (node->isOperation() && node->operation().primitive() == Dag::Operation::EPrimitive::Refine)
					{
						Dag::Refine& refine{ static_cast<Dag::Refine&>(node->operation()) };
						std::unordered_set<Id> vids{};
						{
							mesher.add(refine.parents().single());
							const std::vector<PolyVerts> polys{ Utils::previewRefine(mesher, refine) };
							mesher.remove(refine.parents().single());
							for (const PolyVerts verts : polys)
							{
								for (const Vec& vert : verts)
								{
									const Id vid{ mesher.getVert(vert) };
									if (vid == noId)
									{
										goto nextRefine;
									}
									vids.insert(vid);
								}
							}
						}
						for (const Dag::Element& child : refine.children())
						{
							if (child.children().any([](const Dag::Operation& _childOperation) { return _childOperation.primitive() == Dag::Operation::EPrimitive::Delete; }))
							{
								goto nextRefine;
							}
							const Id childPid{ mesher.elementToPid(child) };
							if (childPid != noId)
							{
								for (const Id vid : mesh.poly_verts_id(mesher.elementToPid(child)))
								{
									if (!vids.contains(vid))
									{
										goto nextRefine;
									}
								}
							}
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
						standardRefines.at(refine.scheme()).push_back(&refine);
					}
				nextRefine:;
				}
				// Subdivide3x3
				bool didSomeSub3x3{};
				do
				{
					didSomeSub3x3 = false;
					std::unordered_set<Dag::Element*> targetCandidates;
					std::vector<Dag::Refine*> sub3x3Refines{ standardRefines.at(Meshing::ERefinementScheme::Subdivide3x3) };
					for (std::size_t i{}; i < sub3x3Refines.size(); i++)
					{
						Dag::Refine& sourceRefine = *sub3x3Refines[i];
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
								Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, Meshing::ERefinementScheme::Subdivide3x3) };
								targetRefine.parents().attach(targetElement);
								Utils::applyRefine(mesher, targetRefine);
								m_operations.push_back({ &targetRefine, &targetElement });
								standardRefines.at(Meshing::ERefinementScheme::Subdivide3x3).push_back(&targetRefine);
								didSomething = true;
								didSomeSub3x3 = true;
							}
						}
						mesher.remove(sourceElement);
					}
				}
				while (didSomeSub3x3);
				// AdapterFaceSubdivide3x3
				{
					for (Dag::Refine* standardRefine : standardRefines.at(Meshing::ERefinementScheme::Subdivide3x3))
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
							Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, Meshing::ERefinementScheme::AdapterFaceSubdivide3x3) };
							targetRefine.parents().attach(targetElement);
							Utils::applyRefine(mesher, targetRefine);
							m_operations.push_back({ &targetRefine, &targetElement });
							didSomething = true;
						}
						mesher.remove(sourceElement);
					}
				}
				// AdapterEdgeSubdivide3x3
				{
					for (Dag::Refine* standardRefine : standardRefines.at(Meshing::ERefinementScheme::Subdivide3x3))
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
								Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, Meshing::ERefinementScheme::AdapterEdgeSubdivide3x3) };
								targetRefine.parents().attach(targetElement);
								Utils::applyRefine(mesher, targetRefine);
								m_operations.push_back({ &targetRefine, &targetElement });
								didSomething = true;
							}
						}
						mesher.remove(sourceElement);
					}
				}
				// Inset
				{
					for (Dag::Refine* standardRefine : standardRefines.at(Meshing::ERefinementScheme::Inset))
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
							Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, Meshing::ERefinementScheme::Inset) };
							targetRefine.parents().attach(targetElement);
							Utils::applyRefine(mesher, targetRefine);
							m_operations.push_back({ &targetRefine, &targetElement });
							didSomething = true;
						}
						mesher.remove(sourceElement);
					}
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

	MakeConforming::MakeConforming()
		: m_operations{}, m_prepared{ false }
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