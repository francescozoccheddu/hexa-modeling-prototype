#include <HMP/Actions/MakeConforming.hpp>

#include <HMP/Meshing/refinementSchemes.hpp>s
#include <HMP/Dag/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <unordered_set>
#include <list>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	MakeConforming::~MakeConforming()
	{
		if (!applied())
		{
			for (const auto [operation, element] : m_operations)
			{
				operation->children().detachAll(true);
				delete operation;
			}
		}
	}

	void MakeConforming::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		if (m_prepared)
		{
			for (const auto [operation, element] : m_operations)
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
				std::list<Dag::Refine*> standardRefines{};
				for (Dag::Node* node : Dag::Utils::descendants(*root()))
				{
					if (node->isOperation() && node->operation().primitive() == Dag::Operation::EPrimitive::Refine)
					{
						Dag::Refine& refine{ static_cast<Dag::Refine&>(node->operation()) };
						std::unordered_set<Id> vids{};
						{
							const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(refine.scheme()) };
							const std::vector<Vec> source{ cpputils::collections::conversions::toVector(refine.parents().single().vertices()) };
							const std::vector<PolyVerts> polys{ refinement.apply(source) };
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
						standardRefines.push_back(&refine);
					}
				nextRefine:;
				}
				// Subdivide3x3
				{
					std::unordered_set<Dag::Element*> targetCandidates;

					for (Dag::Refine* standardRefine : standardRefines)
					{
						if (standardRefine->scheme() != Meshing::ERefinementScheme::Subdivide3x3)
						{
							continue;
						}
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
								Dag::Refine& targetRefine{ Utils::prepareRefine(targetForwardFaceOffset, targetUpFaceOffset, Meshing::ERefinementScheme::Subdivide3x3) };
								targetRefine.parents().attach(targetElement);
								Utils::applyRefine(mesher, targetRefine);
								m_operations.push_back({ &targetRefine, &targetElement });
								standardRefines.push_back(&targetRefine);
								didSomething = true;
							}
						}
						mesher.remove(sourceElement);
					}

				}
				// AdapterFaceSubdivide3x3
				{
					for (Dag::Refine* standardRefine : standardRefines)
					{
						if (standardRefine->scheme() != Meshing::ERefinementScheme::Subdivide3x3)
						{
							continue;
						}
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
					for (Dag::Refine* standardRefine : standardRefines)
					{
						if (standardRefine->scheme() != Meshing::ERefinementScheme::Subdivide3x3)
						{
							continue;
						}
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
								const Id targetForwardFid{ Meshing::Utils::anyFid(mesh, targetPid, sharedEid)};
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
			}
			while (didSomething);
		}
		mesher.updateMesh();
	}

	void MakeConforming::unapply()
	{
		for (const auto [operation, element] : m_operations)
		{
			Utils::unapplyRefine(mesher(), *operation);
		}
		mesher().updateMesh();
	}

	MakeConforming::MakeConforming()
		: m_operations{}, m_prepared{ false }
	{}

}