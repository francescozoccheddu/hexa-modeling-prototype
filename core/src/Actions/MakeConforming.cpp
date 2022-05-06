#include <HMP/Actions/MakeConforming.hpp>

#include <HMP/Meshing/refinementSchemes.hpp>s
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cinolib/stl_container_utilities.h>

namespace HMP::Actions
{

	MakeConforming::MakeConforming() {}


	void MakeConforming::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };

		std::vector<Dag::Refine*> refines{};
		std::unordered_set<Id> removedPids{};

		for (Dag::Node* node : Dag::Utils::descendants(*root()))
		{
			if (node->isOperation() && node->operation().primitive() == Dag::Operation::EPrimitive::Refine)
			{
				Dag::Refine& refine{ static_cast<Dag::Refine&>(node->operation()) };
				if (refine.needsTopologyFix())
				{
					refines.push_back(&refine);
				}
			}
		}

		// Subdivide3x3
		{
			std::deque<Dag::Refine*> sources(refines.begin(), refines.end());
			std::set<Dag::Element*> targetCandidates;

			while (!sources.empty())
			{
				Dag::Refine& sourceRefine = *sources.front();
				sources.pop_front();

				Dag::Element& sourceElement = sourceRefine.parents().single();
				mesher.add(sourceElement);

				for (Id targetPid : mesh.adj_p2p(mesher.elementToPid(sourceElement)))
				{
					Dag::Element& targetElement = mesher.pidToElement(targetPid);
					if (!targetCandidates.insert(&targetElement).second)
					{
						Dag::Refine& targetRefine{ *new Dag::Refine{} };
						m_operations.push_back(&targetRefine);
						targetRefine.scheme() = Meshing::ERefinementScheme::Subdivide3x3;
						targetRefine.needsTopologyFix() = true;
						targetElement.children().attach(targetRefine);
						const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(Meshing::ERefinementScheme::Subdivide3x3) };
						const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector(targetElement.vertices())) };
						for (const PolyVerts& verts : polys)
						{
							Dag::Element& child{ *new Dag::Element{} };
							child.vertices() = verts;
							targetRefine.children().attach(child);
							mesher.add(child);
						}
						removedPids.insert(mesher.elementToPid(targetElement));
						refines.push_back(&targetRefine);
						sources.push_back(&targetRefine);
						m_fixedRefines.insert(&sourceRefine);
						sourceRefine.needsTopologyFix() = false;
					}
				}
				removedPids.insert(mesher.elementToPid(sourceElement));
			}

		}

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

	}

	void MakeConforming::unapply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		for (Dag::Refine* operation : m_operations)
		{
			for (Dag::Element& child : operation->children())
			{
				mesher.remove(child);
			}
			mesher.add(operation->parents().single());
			operation->children().detachAll(true);
			delete operation;
		}
		for (Dag::Refine* fixedRefine : m_fixedRefines)
		{
			fixedRefine->needsTopologyFix() = true;
		}
		m_operations.clear();
	}

}