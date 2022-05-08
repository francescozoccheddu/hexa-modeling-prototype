#include <HMP/Actions/MakeConforming.hpp>

#include <HMP/Meshing/refinementSchemes.hpp>s
#include <HMP/Dag/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <unordered_set>
#include <list>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Actions/Refine.hpp>

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
		if (m_prepared)
		{
			for (const auto [operation, element] : m_operations)
			{
				Refine::applyRefine(mesher(), *element, *operation);
			}
		}
		else
		{
			m_prepared = true;
			Meshing::Mesher& mesher{ this->mesher() };
			const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
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
						for (const Id vid : mesh.poly_verts_id(mesher.elementToPid(child)))
						{
							if (!vids.contains(vid))
							{
								goto nextRefine;
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
					for (const Id targetPid : mesh.adj_p2p(mesher.elementToPid(sourceElement)))
					{
						Dag::Element& targetElement = mesher.pidToElement(targetPid);
						if (!targetCandidates.insert(&targetElement).second)
						{
							Dag::Refine& targetRefine{ Refine::prepareRefine(0, Meshing::ERefinementScheme::Subdivide3x3) };
							Refine::applyRefine(mesher, targetElement, targetRefine);
							m_operations.push_back({ &targetRefine, &targetElement });
							standardRefines.push_back(&targetRefine);
						}
					}
					mesher.remove(sourceElement);
				}

			}
			/*
			// InterfaceFace
			{
				std::deque<Dag::Refine*> sources(refines.begin(), refines.end());
				while (!sources.empty())
				{
					Dag::Refine& sourceRefine = *sources.front();
					sources.pop_front();

					Dag::Element& sourceElement = sourceRefine.parents().single();
					mesher.add(sourceElement);

					for (Id targetPid : mesh.adj_p2p(mesher.elementToPid(sourceElement)))
					{
						if (removedPids.contains(targetPid))
						{
							continue;
						}
						const Id fid = mesh.poly_shared_face(mesher.elementToPid(sourceElement), targetPid);
						if (targetPid == mesher.elementToPid(sourceElement) || fid == -1)
						{
							continue;
						}
						Dag::Element& targetElement = mesher.pidToElement(targetPid);
						Dag::Refine& targetRefine{ *new Dag::Refine{} };
						m_operations.push_back(&targetRefine);
						targetRefine.scheme() = Meshing::ERefinementScheme::InterfaceFace;
						targetRefine.needsTopologyFix() = false;
						targetElement.children().attach(targetRefine);
						const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(Meshing::ERefinementScheme::InterfaceFace) };
						const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector(Meshing::Utils::polyVertsFromFace(mesh, mesher.elementToPid(targetElement), fid))) };
						for (const PolyVerts& verts : polys)
						{
							Dag::Element& child{ *new Dag::Element{} };
							child.vertices() = verts;
							targetRefine.children().attach(child);
							mesher.add(child);
						}
						removedPids.insert(mesher.elementToPid(targetElement));
						m_fixedRefines.insert(&sourceRefine);
						sourceRefine.needsTopologyFix() = false;
					}
					removedPids.insert(mesher.elementToPid(sourceElement));
				}
			}

			// InterfaceEdge
			{
				std::deque<Dag::Refine*> sources(refines.begin(), refines.end());
				while (!sources.empty())
				{
					Dag::Refine& sourceRefine = *sources.front();
					sources.pop_front();

					Dag::Element& sourceElement = sourceRefine.parents().single();
					mesher.add(sourceElement);

					for (Id targetEid : mesh.adj_p2e(mesher.elementToPid(sourceElement)))
					{
						for (Id targetPid : mesh.adj_e2p(targetEid))
						{
							if (removedPids.contains(targetPid))
							{
								continue;
							}
							if (targetPid == mesher.elementToPid(sourceElement) || mesh.poly_shared_face(targetPid, mesher.elementToPid(sourceElement)) != -1)
							{
								continue;
							}
							Dag::Element& targetElement = mesher.pidToElement(targetPid);
							Dag::Refine& targetRefine{ *new Dag::Refine{} };
							m_operations.push_back(&targetRefine);
							targetRefine.scheme() = Meshing::ERefinementScheme::InterfaceEdge;
							targetRefine.needsTopologyFix() = false;
							targetElement.children().attach(targetRefine);
							const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(Meshing::ERefinementScheme::InterfaceEdge) };
							const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector(Meshing::Utils::polyVertsFromEdge(mesh,mesher.elementToPid(targetElement), targetEid))) };
							for (const PolyVerts& verts : polys)
							{
								Dag::Element& child{ *new Dag::Element{} };
								child.vertices() = verts;
								targetRefine.children().attach(child);
								mesher.add(child);
							}
							removedPids.insert(mesher.elementToPid(targetElement));
							m_fixedRefines.insert(&sourceRefine);
							sourceRefine.needsTopologyFix() = false;
						}
					}
					removedPids.insert(mesher.elementToPid(sourceElement));
				}
			}

			{
				std::vector<Id> pids(removedPids.begin(), removedPids.end());
				std::sort(pids.begin(), pids.end(), std::greater<Id>{});
				for (Id pid : pids)
				{
					mesher.remove(mesher.pidToElement(pid));
				}
			}
		*/
		}
		mesher().updateMesh();
	}

	void MakeConforming::unapply()
	{
		for (const auto [operation, element] : m_operations)
		{
			Refine::unapplyRefine(mesher(), *operation);
		}
		mesher().updateMesh();
	}

	MakeConforming::MakeConforming()
		: m_operations{}, m_prepared{ false }
	{}

}