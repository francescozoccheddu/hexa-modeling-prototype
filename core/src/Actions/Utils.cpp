#include <HMP/Actions/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <algorithm>

namespace HMP::Actions::Utils
{

	Dag::Extrude& prepareExtrude(Id _vertOffset, bool _clockwise, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets)
	{
		Dag::Extrude& extrude{ *new Dag::Extrude{} };
		extrude.vertOffset() = _vertOffset;
		extrude.clockwise() = _clockwise;
		extrude.faceOffsets() = _faceOffsets;
		switch (_faceOffsets.size())
		{
			case 1:
				extrude.source() = Dag::Extrude::ESource::Face;
				break;
			case 2:
				extrude.source() = Dag::Extrude::ESource::Edge;
				break;
			case 3:
				extrude.source() = Dag::Extrude::ESource::Vertex;
				break;
			default:
				throw std::domain_error{ "empty" };
		}
		extrude.children().attach(*new Dag::Element{});
		return extrude;
	}

	PolyVerts extrudeFace(const Meshing::Mesher::Mesh& _mesh, const Id _pid, const Id _fid, const FaceVertIds& _vids)
	{
		const FaceVerts faceVerts{ Meshing::Utils::verts(_mesh, _vids) };
		const Real avgEdgeLength{ Meshing::Utils::avgFidEdgeLength(_mesh, _fid) };
		PolyVerts verts;
		std::copy(faceVerts.begin(), faceVerts.end(), verts.begin());
		const Vec faceNormal = _mesh.poly_face_normal(_pid, _fid);
		int i{ 4 };
		for (const Vec& faceVert : faceVerts)
		{
			verts[i++] = faceVert + faceNormal * avgEdgeLength;
		}
		return verts;
	}

	PolyVerts shapeFaceExtrude(const Meshing::Mesher::Mesh& _mesh, const Id _pid, const Id _fid, Id _firstVid)
	{
		const FaceVertIds faceVids{ Meshing::Utils::pidFidVidsByFirstVid(_mesh, _pid, _fid, _firstVid) };
		return extrudeFace(_mesh, _pid, _fid, faceVids);
	}

	PolyVerts shapeEdgeExtrude(const Meshing::Mesher::Mesh& _mesh, const std::array<Id, 2>& _pids, const std::array<Id, 2>& _fids, Id _firstVid, bool _clockwise)
	{
		const std::array<PolyVerts, 2> verts{
			cpputils::range::of(_fids).zip(_pids).map([&](const auto& _fidAndPid) {
				const auto& [fid, pid] {_fidAndPid};
				return shapeFaceExtrude(_mesh, pid, fid, _firstVid);
			}).toArray()
		};
		return _clockwise
			? PolyVerts{
				verts[0][0], verts[0][1], verts[0][2], verts[0][3],
				verts[1][3], (verts[0][5] + verts[1][7]) / 2, (verts[0][6] + verts[1][6]) / 2, verts[1][2]
		}
			: PolyVerts{
				verts[0][0], verts[0][1], verts[0][2], verts[0][3],
				verts[1][1], verts[1][2], (verts[0][6] + verts[1][6]) / 2, (verts[0][7] + verts[1][5]) / 2
		};
	}

	PolyVerts shapeVertexExtrude(const Meshing::Mesher::Mesh& _mesh, const std::array<Id, 3>& _pids, const std::array<Id, 3>& _fids, Id _firstVid, bool _clockwise)
	{
		const std::array<PolyVerts, 3> verts{
			cpputils::range::of(_fids).zip(_pids).map([&](const auto& _fidAndPid) {
				const auto& [fid, pid] {_fidAndPid};
				return shapeFaceExtrude(_mesh, pid, fid, _firstVid);
			}).toArray()
		};
		return _clockwise
			? PolyVerts{
				verts[0][0], verts[0][1], verts[0][2], verts[0][3],
				verts[2][1], verts[2][2], (verts[0][6] + verts[1][6] + verts[2][6]) / 3, verts[1][2]
		}
			: PolyVerts{
				verts[0][0], verts[0][1], verts[0][2], verts[0][3],
				verts[1][1], verts[1][2], (verts[0][6] + verts[1][6] + verts[2][6]) / 3, verts[2][2]
		};
	}

	PolyVerts shapeExtrude(const Meshing::Mesher::Mesh& _mesh, const cpputils::collections::FixedVector<Id, 3>& _pids, const cpputils::collections::FixedVector< Id, 3>& _fids, Id _firstVid, bool _clockwise)
	{
		switch (_pids.size())
		{
			case 1:
				return shapeFaceExtrude(_mesh, _pids[0], _fids[0], _firstVid);
			case 2:
				return shapeEdgeExtrude(_mesh, cpputils::range::of(_pids).toArray<2>(), cpputils::range::of(_fids).toArray<2>(), _firstVid, _clockwise);
			case 3:
				return shapeVertexExtrude(_mesh, cpputils::range::of(_pids).toArray<3>(), cpputils::range::of(_fids).toArray<3>(), _firstVid, _clockwise);
			default:
				throw std::logic_error{ "empty" };
		}
	}

	PolyVerts shapeExtrude(const Meshing::Mesher& _mesher, const cpputils::collections::FixedVector<const Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Id _vertOffset, bool _clockwise)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const cpputils::collections::FixedVector<Id, 3> pids{ cpputils::range::of(_elements).map([&](const Dag::Element* _parent) {
			return _mesher.elementToPid(*_parent);
		}).toFixedVector<3>() };
		const cpputils::collections::FixedVector<Id, 3> fids{ cpputils::range::of(_faceOffsets).zip(pids).map([&](const auto& foAndPid) {
			const auto [fo, pid] {foAndPid};
			return mesh.poly_face_id(pid, fo);
		}).toFixedVector<3>() };
		const Id vid{ mesh.poly_vert_id(pids[0], _vertOffset) };
		return shapeExtrude(mesh, pids, fids, vid, _clockwise);
	}

	PolyVerts shapeExtrude(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude)
	{
		return shapeExtrude(_mesher, _extrude.parents().address().immutable().toFixedVector<3>(), _extrude.faceOffsets(), _extrude.vertOffset(), _extrude.clockwise());
	}

	void applyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _extrude)
	{
		Dag::Element& child{ _extrude.children().single() };
		child.vertices() = shapeExtrude(_mesher, _extrude);
		_mesher.add(child);
	}

	void unapplyExtrude(Meshing::Mesher& _mesher, Dag::Extrude& _extrude, bool _detach)
	{
		if (_detach)
		{
			_extrude.parents().detachAll(false);
		}
		_mesher.remove(_extrude.children().single());
	}

}