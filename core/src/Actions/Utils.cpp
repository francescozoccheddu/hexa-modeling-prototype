#include <HMP/Actions/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <cpputils/collections/zip.hpp>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <algorithm>

namespace HMP::Actions::Utils
{

	Dag::Refine& prepareRefine(Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme, std::size_t _depth)
	{
		if (_depth < 1 || _depth > 3)
		{
			throw std::logic_error{ "depth must fall in range [1,3]" };
		}
		Dag::Refine& refine{ *new Dag::Refine{} };
		refine.scheme() = _scheme;
		refine.forwardFaceOffset() = _forwardFaceOffset;
		refine.upFaceOffset() = _upFaceOffset;
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(_scheme) };
		for (std::size_t i{ 0 }; i < refinement.polyCount(); i++)
		{
			Dag::Element& child{ *new Dag::Element{} };
			if (_depth > 1)
			{
				child.children().attach(prepareRefine(_forwardFaceOffset, _upFaceOffset, _scheme, _depth - 1));
			}
			refine.children().attach(child);
		}
		return refine;
	}

	std::vector<PolyVerts> previewRefine(const Meshing::Mesher& _mesher, const Dag::Refine& _refine)
	{
		const Dag::Element& element{ _refine.parents().single() };
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(_refine.scheme()) };
		if (refinement.polyCount() != _refine.children().size())
		{
			throw std::logic_error{ "wrong number of children" };
		}
		const Id pid{ _mesher.elementToPid(element) };
		if (pid == noId)
		{
			throw std::logic_error{ "not an element" };
		}
		const Id forwardFid{ mesh.poly_face_id(pid, _refine.forwardFaceOffset()) };
		const Id upFid{ mesh.poly_face_id(pid, _refine.upFaceOffset()) };
		const Id upEid{ mesh.face_shared_edge(forwardFid, upFid) };
		const PolyVertIds sourceVids{ Meshing::Utils::polyVids(mesh, pid, forwardFid, upEid) };
		const PolyVerts source{ Meshing::Utils::verts(mesh, sourceVids) };
		const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector(source)) };
		return polys;
	}

	void applyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const std::vector<PolyVerts> polys{ previewRefine(_mesher, _refine) };
		for (const auto& [child, verts] : cpputils::collections::zip(_refine.children(), polys))
		{
			child.vertices() = verts;
			_mesher.add(child);
		}
		_mesher.remove(_refine.parents().single());
	}

	void applyRefineRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		applyRefine(_mesher, _refine);
		for (Dag::Element& child : _refine.children())
		{
			const auto refineIt{ child.children().singleIt([](const Dag::Operation& _op) {return _op.primitive() == Dag::Operation::EPrimitive::Refine; }) };
			if (refineIt != child.children().end())
			{
				applyRefineRecursive(_mesher, static_cast<Dag::Refine&>(*refineIt));
			}
		}
	}

	void unapplyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine, bool _detach)
	{
		for (Dag::Element& child : _refine.children())
		{
			_mesher.remove(child);
		}
		_mesher.add(_refine.parents().single());
		if (_detach)
		{
			_refine.parents().detachAll(false);
		}
	}

	void unapplyRefineRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine, bool _detach)
	{
		for (Dag::Element& child : _refine.children())
		{
			const auto refineIt{ child.children().singleIt([](const Dag::Operation& _op) {return _op.primitive() == Dag::Operation::EPrimitive::Refine; }) };
			if (refineIt != child.children().end())
			{
				unapplyRefineRecursive(_mesher, static_cast<Dag::Refine&>(*refineIt), false);
			}
		}
		unapplyRefine(_mesher, _refine, _detach);
	}

	Dag::Delete& prepareDelete()
	{
		return *new Dag::Delete{};
	}

	void applyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete)
	{
		Dag::Element& element{ _delete.parents().single() };
		_mesher.remove(element);
	}

	void unapplyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete, bool _detach)
	{
		_mesher.add(_delete.parents().single());
		if (_detach)
		{
			_delete.parents().detachAll(false);
		}
	}

	Dag::Extrude& prepareExtrude(Id _forwardFaceOffset, Id _upFaceOffset)
	{
		Dag::Extrude& extrude{ *new Dag::Extrude{} };
		extrude.forwardFaceOffset() = _forwardFaceOffset;
		extrude.upFaceOffset() = _upFaceOffset;
		extrude.children().attach(*new Dag::Element{});
		return extrude;
	}

	void applyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _extrude)
	{
		Dag::Element& element{ _extrude.parents().single() };
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Id pid{ _mesher.elementToPid(element) };
		const Id forwardFid{ mesh.poly_face_id(pid, _extrude.forwardFaceOffset()) };
		const Id upFid{ mesh.poly_face_id(pid, _extrude.upFaceOffset()) };
		const Id upEid{ mesh.face_shared_edge(forwardFid, upFid) };
		const FaceVertIds faceVids{ Meshing::Utils::faceVids(mesh, pid, forwardFid, upEid, true) };
		const FaceVerts faceVerts{ Meshing::Utils::verts(mesh, faceVids) };
		Real avgFaceEdgeLength{};
		{
			const std::vector<Id> faceEids{ mesh.adj_f2e(forwardFid) };
			for (const Id eid : faceEids)
			{
				avgFaceEdgeLength += mesh.edge_length(eid);
			}
			avgFaceEdgeLength /= faceEids.size();
		}
		PolyVerts& verts{ _extrude.children().single().vertices() };
		std::copy(faceVerts.begin(), faceVerts.end(), verts.begin());
		const Vec faceNormal = mesh.poly_face_normal(pid, forwardFid);
		int i{ 4 };
		for (const Vec& faceVert : faceVerts)
		{
			verts[i++] = faceVert + faceNormal * avgFaceEdgeLength;
		}
		_mesher.add(_extrude.children().single());
	}

	void unapplyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _extrude, bool _detach)
	{
		if (_detach)
		{
			_extrude.parents().detachAll(false);
		}
		_mesher.remove(_extrude.children().single());
	}

	void applyTree(Meshing::Mesher& _mesher, Dag::Node& _node)
	{
		for (Dag::Node* node : Dag::Utils::descendants(_node))
		{
			if (node->isOperation())
			{
				switch (node->operation().primitive())
				{
					case Dag::Operation::EPrimitive::Delete:
						applyDelete(_mesher, static_cast<Dag::Delete&>(*node));
						break;
					case Dag::Operation::EPrimitive::Extrude:
						applyExtrude(_mesher, static_cast<Dag::Extrude&>(*node));
						break;
					case Dag::Operation::EPrimitive::Refine:
						applyRefine(_mesher, static_cast<Dag::Refine&>(*node));
						break;
					default:
						throw std::domain_error{ "unexpected primitive" };
				}
			}
		}
	}

}