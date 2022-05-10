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

	Dag::Refine& prepareRefine(Id _forwardFaceOffset, Id _upFaceOffset, Meshing::ERefinementScheme _scheme)
	{
		Dag::Refine& refine{ *new Dag::Refine{} };
		refine.scheme() = _scheme;
		refine.forwardFaceOffset() = _forwardFaceOffset;
		refine.upFaceOffset() = _upFaceOffset;
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(_scheme) };
		for (std::size_t i{ 0 }; i < refinement.polyCount(); i++)
		{
			refine.children().attach(*new Dag::Element{});
		}
		return refine;
	}

	void applyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		Dag::Element& element{ _refine.parents().single() };
		for (const Dag::Operation& child : element.children())
		{
			if (child != _refine && child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
		}
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(_refine.scheme()) };
		if (refinement.polyCount() != _refine.children().size())
		{
			throw std::logic_error{ "wrong number of children" };
		}
		const Id pid{ _mesher.elementToPid(element) };
		const Id forwardFid{ mesh.poly_face_id(pid, _refine.forwardFaceOffset()) };
		const Id upFid{ mesh.poly_face_id(pid, _refine.upFaceOffset()) };
		const Id upEid{ mesh.face_shared_edge(forwardFid, upFid) };
		const PolyVertIds sourceVids{ Meshing::Utils::polyVids(mesh, pid, forwardFid, upEid) };
		const PolyVerts source{ Meshing::Utils::verts(mesh, sourceVids) };
		const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector(source)) };
		for (const auto& [child, verts] : cpputils::collections::zip(_refine.children(), polys))
		{
			child.vertices() = verts;
			_mesher.add(child);
		}
		_mesher.remove(element);
	}

	void unapplyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine, bool _detach)
	{
		for (Dag::Element& child : _refine.children())
		{
			_mesher.remove(child);
		}
		_mesher.add(_refine.parents().single());
		_refine.parents().detachAll(false);
	}

	Dag::Delete& prepareDelete()
	{
		return *new Dag::Delete{};
	}

	void applyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete)
	{
		Dag::Element& element{ _delete.parents().single() };
		if (_mesher.mesh().num_polys() == 1)
		{
			throw std::logic_error{ "cannot delete the only active element" };
		}
		for (const Dag::Operation& child : element.children())
		{
			if (child != _delete && child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
		}
		_mesher.remove(element);
	}

	void unapplyDelete(Meshing::Mesher& _mesher, Dag::Delete& _delete, bool _detach)
	{
		_mesher.add(_delete.parents().single());
		_delete.parents().detachAll(false);
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
		for (const Dag::Operation& child : element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
			if (child != _extrude && static_cast<const Dag::Extrude&>(child).forwardFaceOffset() == _extrude.forwardFaceOffset())
			{
				throw std::logic_error{ "element already has equivalent child" };
			}
		}
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
		const Vec faceNormal = mesh.face_data(forwardFid).normal;
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