#include <HMP/Actions/MakeConforming.hpp>

#include <HMP/Refinement/Schemes.hpp>
#include <HMP/Dag/Utils.hpp>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <optional>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Refinement/Utils.hpp>

namespace HMP::Actions
{

	using Refinement::EScheme;

	MakeConforming::RefinementMap MakeConforming::findStandardRefinements(const std::set<Refinement::EScheme>& _schemes)
	{
		std::unordered_map<EScheme, std::vector<Dag::Refine*>> standardRefines{};
		// map scheme -> refinements
		standardRefines.reserve(_schemes.size());
		for (EScheme scheme : _schemes)
		{
			standardRefines.insert({ scheme, {} });
		}
		for (Dag::Node* node : Dag::Utils::descendants(*root()))
		{
			if (node->isOperation() && node->operation().primitive == Dag::Operation::EPrimitive::Refine)
			{
				Dag::Refine& refine{ static_cast<Dag::Refine&>(node->operation()) };
				const auto mapIt{ standardRefines.find(refine.scheme) };
				if (mapIt != standardRefines.end())
				{
					// add to the list of standard refinements
					mapIt->second.push_back(&refine);
				}
			}
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
			Dag::Element& refEl = ref.parents.single();
			const Id refPid{ refEl.pid };
			// for each adjacent poly candPid
			for (const Id candPid : mesh.adj_p2p(refPid))
			{
				// skip if self-adjacent
				if (candPid == refPid)
				{
					continue;
				}
				const Id sharedFid = static_cast<Id>(mesh.poly_shared_face(refPid, candPid));
				// skip if they do not share a face
				if (sharedFid == noId)
				{
					continue;
				}
				// skip if the shared face is not the refined one
				if (sharedFid != mesh.poly_face_id(refPid, ref.forwardFi))
				{
					continue;
				}
				// get the orientation right
				Dag::Element& candEl = mesher.element(candPid);
				const Id candForwardFid{ sharedFid };
				const Id candForwardFaceOffset{ mesh.poly_face_offset(candPid, candForwardFid) };
				const Id candUpFid{ Meshing::Utils::adjFidInPidByFidAndEid(mesh, candPid, candForwardFid, mesh.face_edge_id(candForwardFid, 0)) };
				const Id candUpFaceOffset{ mesh.poly_face_offset(candPid, candUpFid) };
				// apply the refinement
				Dag::Refine& adapterRef{ Refinement::Utils::prepare(candForwardFaceOffset, candUpFaceOffset, EScheme::Inset) };
				adapterRef.parents.attach(candEl);
				Refinement::Utils::apply(mesher, adapterRef);
				m_operations.push_back({ &adapterRef, &candEl });
				_insets.push_back(&adapterRef);
			}
		}
	}

	void MakeConforming::installSubdivide3x3Adapters(std::vector<Dag::Refine*>& _sub3x3s)
	{
		Meshing::Mesher& mesher{ this->mesher() };
		bool didSomething{ false };
		do
		{
			Refinement::Utils::Sub3x3AdapterCandidateSet set{};
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
				const Refinement::Utils::Sub3x3AdapterCandidate candidate{ set.pop() };
				Dag::Refine& adapterRefine{ candidate.prepareAdapter() };
				adapterRefine.parents.attach(candidate.element());
				Refinement::Utils::apply(mesher, adapterRefine);
				m_operations.push_back({ &adapterRefine, &candidate.element() });
				// if the refinement is a new Subdivide3x3, then add it to the set
				if (candidate.scheme() == EScheme::Subdivide3x3)
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
				operation->parents.attach(*element);
				Refinement::Utils::apply(mesher, *operation);
			}
		}
		else
		{
			m_prepared = true;
			RefinementMap refinesMap{ findStandardRefinements({EScheme::Subdivide3x3, EScheme::Inset }) };
			installSubdivide3x3Adapters(refinesMap.at(EScheme::Subdivide3x3));
			installInsetAdapters(refinesMap.at(EScheme::Inset));
		}
		mesher.updateMesh();
	}

	void MakeConforming::unapply()
	{
		for (auto it{ m_operations.rbegin() }; it != m_operations.rend(); ++it)
		{
			auto& [operation, element] {*it};
			//Refinement::Utils::unapply(mesher(), *operation);
		}
		mesher().updateMesh();
	}

	MakeConforming::MakeConforming(): m_operations{}, m_prepared{ false }
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