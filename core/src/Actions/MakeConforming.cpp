#include <HMP/Actions/MakeConforming.hpp>

#include <HMP/Meshing/refinementSchemes.hpp>s
#include <HMP/grid.hpp>
#include <HMP/Dag/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cinolib/stl_container_utilities.h>

namespace HMP::Actions
{

	MakeConforming::MakeConforming() {}


	void MakeConforming::apply()
	{
		Grid& grid{ this->grid() };
		Grid::Mesh& mesh{ grid.mesh() };

		std::vector<Dag::Refine*> refines{};
		std::unordered_set<Id> removedPids{};

		for (Dag::Node* node : Dag::Utils::descendants(grid.root()))
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
				grid.addPoly(sourceElement);

				for (Id targetPid : mesh.adj_p2p(sourceElement.pid()))
				{
					Dag::Element& targetElement = *mesh.poly_data(targetPid).element;
					if (!targetCandidates.insert(&targetElement).second)
					{
						Dag::Refine& targetRefine{ *new Dag::Refine{} };
						m_operations.push_back(&targetRefine);
						targetRefine.scheme() = Meshing::ERefinementScheme::Subdivide3x3;
						targetRefine.needsTopologyFix() = true;
						targetElement.children().attach(targetRefine);
						const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(Meshing::ERefinementScheme::Subdivide3x3) };
						const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector(targetElement.vertices())) };
						for (std::size_t i{ 0 }; i < refinement.polyCount(); i++)
						{
							Dag::Element& child{ *new Dag::Element{} };
							targetRefine.children().attach(child);
							grid.addPoly(polys[i], child);
						}
						removedPids.insert(targetElement.pid());
						refines.push_back(&targetRefine);
						sources.push_back(&targetRefine);
						m_fixedRefines.insert(&sourceRefine);
						sourceRefine.needsTopologyFix() = false;
					}
				}
				removedPids.insert(sourceElement.pid());
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
				grid.addPoly(sourceElement);

				for (Id targetPid : mesh.adj_p2p(sourceElement.pid()))
				{
					if (removedPids.contains(targetPid))
					{
						continue;
					}
					const Id fid = mesh.poly_shared_face(sourceElement.pid(), targetPid);
					if (targetPid == sourceElement.pid() || fid == -1)
					{
						continue;
					}
					Dag::Element& targetElement = *mesh.poly_data(targetPid).element;
					Dag::Refine& targetRefine{ *new Dag::Refine{} };
					m_operations.push_back(&targetRefine);
					targetRefine.scheme() = Meshing::ERefinementScheme::InterfaceFace;
					targetRefine.needsTopologyFix() = false;
					targetElement.children().attach(targetRefine);
					const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(Meshing::ERefinementScheme::InterfaceFace) };
					const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector<const std::array<Vec, 8>>(Meshing::Utils::polyVertsFromFace(mesh, targetElement.pid(), fid))) };
					for (std::size_t i{ 0 }; i < refinement.polyCount(); i++)
					{
						Dag::Element& child{ *new Dag::Element{} };
						targetRefine.children().attach(child);
						grid.addPoly(polys[i], child);
					}
					removedPids.insert(targetElement.pid());
					m_fixedRefines.insert(&sourceRefine);
					sourceRefine.needsTopologyFix() = false;
				}
				removedPids.insert(sourceElement.pid());
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
				grid.addPoly(sourceElement);

				for (Id targetEid : mesh.adj_p2e(sourceElement.pid()))
				{
					for (Id targetPid : mesh.adj_e2p(targetEid))
					{
						if (removedPids.contains(targetPid))
						{
							continue;
						}
						if (targetPid == sourceElement.pid() || mesh.poly_shared_face(targetPid, sourceElement.pid()) != -1)
						{
							continue;
						}
						Dag::Element& targetElement = *mesh.poly_data(targetPid).element;
						Dag::Refine& targetRefine{ *new Dag::Refine{} };
						m_operations.push_back(&targetRefine);
						targetRefine.scheme() = Meshing::ERefinementScheme::InterfaceEdge;
						targetRefine.needsTopologyFix() = false;
						targetElement.children().attach(targetRefine);
						const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(Meshing::ERefinementScheme::InterfaceEdge) };
						const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector<const std::array<Vec, 8>>(Meshing::Utils::polyVertsFromEdge(mesh,targetElement.pid(), targetEid))) };
						for (std::size_t i{ 0 }; i < refinement.polyCount(); i++)
						{
							Dag::Element& child{ *new Dag::Element{} };
							targetRefine.children().attach(child);
							grid.addPoly(polys[i], child);
						}
						removedPids.insert(targetElement.pid());
						m_fixedRefines.insert(&sourceRefine);
						sourceRefine.needsTopologyFix() = false;
					}
				}
				removedPids.insert(sourceElement.pid());
			}
		}

		{
			std::vector<Id> pids(removedPids.begin(), removedPids.end());
			std::sort(pids.begin(), pids.end(), std::greater<Id>{});
			for (Id pid : pids)
			{
				grid.removePoly(pid);
			}
		}

	}

	void MakeConforming::unapply()
	{
		Grid& grid{ this->grid() };
		for (Dag::Refine* operation : m_operations)
		{
			for (Dag::Element& child : operation->children())
			{
				grid.removePoly(child.pid());
			}
			grid.addPoly(operation->parents().single());
			delete operation;
		}
		for (Dag::Refine* fixedRefine : m_fixedRefines)
		{
			fixedRefine->needsTopologyFix() = true;
		}
		m_operations.clear();
	}

}