#include <HMP/Actions/ExtrudeUtils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/join.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <cassert>

namespace HMP::Actions::ExtrudeUtils
{

	Dag::Extrude& prepare(const cpputils::collections::FixedVector<I, 3>& _fis, I _firstVi, bool _clockwise)
	{
		Dag::Extrude& extrude{ *new Dag::Extrude{} };
		extrude.firstVi = _firstVi;
		extrude.clockwise = _clockwise;
		extrude.fis = _fis;
		switch (_fis.size())
		{
			case 1:
				extrude.source = Dag::Extrude::ESource::Face;
				break;
			case 2:
				extrude.source = Dag::Extrude::ESource::Edge;
				break;
			case 3:
				extrude.source = Dag::Extrude::ESource::Vertex;
				break;
			default:
				assert(false);
		}
		extrude.children().attach(*new Dag::Element{});
		return extrude;
	}

	FaceVerts extrudeFace(const Meshing::Mesher::Mesh& _mesh, const Id _pid, const Id _fid, const FaceVertIds& _vids)
	{
		const FaceVerts inVerts{ Meshing::Utils::verts(_mesh, _vids) };
		const Real avgEdgeLength{ Meshing::Utils::avgFidEdgeLength(_mesh, _fid) };
		const Vec faceNormal = _mesh.poly_face_normal(_pid, _fid);
		FaceVerts outVerts;
		for (const auto& [in, out] : cpputils::range::zip(inVerts, outVerts))
		{
			out = in + faceNormal * avgEdgeLength;
		}
		return outVerts;
	}

	PolyVertIds applyFaceExtrude(const Meshing::Mesher::Mesh& _mesh, const Id _pid, const Id _fid, Id _firstVid, std::vector<Vec>& _newVerts)
	{
		const FaceVertIds faceVids{ Meshing::Utils::pidFidVidsByFirstVid(_mesh, _pid, _fid, _firstVid) };
		const FaceVerts newVerts{ extrudeFace(_mesh, _pid, _fid, faceVids) };
		PolyVertIds vids;
		std::copy(faceVids.begin(), faceVids.end(), vids.begin());
		const Id firstNewVid{ _mesh.num_verts() + toId(_newVerts.size()) };
		_newVerts.insert(_newVerts.end(), newVerts.begin(), newVerts.end());
		for (I i{}; i < 4; i++)
		{
			vids[i + 4] = firstNewVid + toId(i);
		}
		return vids;
	}

	PolyVertIds applyEdgeExtrude(const Meshing::Mesher::Mesh& _mesh, const std::array<Id, 2>& _pids, const std::array<Id, 2>& _fids, Id _firstVid, bool _clockwise, std::vector<Vec>& _newVerts)
	{
		const std::array<FaceVertIds, 2> faceVids{
			cpputils::range::zip(_fids, _pids).map([&](const auto& _fidAndPid) {
				const auto& [fid, pid] {_fidAndPid};
				return Meshing::Utils::pidFidVidsByFirstVid(_mesh, pid, fid, _firstVid);
			}).toArray()
		};
		const std::array<FaceVerts, 2> newFaceVerts{
			cpputils::range::zip(_fids, _pids, faceVids).map([&](const auto& _fidAndPidAndVids) {
				const auto& [fid, pid, vids] {_fidAndPidAndVids};
				return extrudeFace(_mesh, pid, fid, vids);
			}).toArray()
		};
		const Id firstNewVid{ _mesh.num_verts() + toId(_newVerts.size()) };
		if (_clockwise)
		{
			_newVerts.push_back((newFaceVerts[0][1] + newFaceVerts[1][3]) / 2);
			_newVerts.push_back((newFaceVerts[0][2] + newFaceVerts[1][2]) / 2);
			return {
				faceVids[0][0], faceVids[0][1], faceVids[0][2], faceVids[0][3],
				faceVids[1][3], firstNewVid + 0, firstNewVid + 1, faceVids[1][2]
			};
		}
		else
		{
			_newVerts.push_back((newFaceVerts[0][2] + newFaceVerts[1][2]) / 2);
			_newVerts.push_back((newFaceVerts[0][3] + newFaceVerts[1][1]) / 2);
			return {
				faceVids[0][0], faceVids[0][1], faceVids[0][2], faceVids[0][3],
				faceVids[1][1], faceVids[1][2], firstNewVid + 0, firstNewVid + 1
			};
		}
	}

	PolyVertIds applyVertexExtrude(const Meshing::Mesher::Mesh& _mesh, const std::array<Id, 3>& _pids, const std::array<Id, 3>& _fids, Id _firstVid, bool _clockwise, std::vector<Vec>& _newVerts)
	{
		const std::array<FaceVertIds, 3> faceVids{
			cpputils::range::zip(_fids,_pids).map([&](const auto& _fidAndPid) {
				const auto& [fid, pid] {_fidAndPid};
				return Meshing::Utils::pidFidVidsByFirstVid(_mesh, pid, fid, _firstVid);
			}).toArray()
		};
		const std::array<FaceVerts, 3> newFaceVerts{
			cpputils::range::zip(_fids, _pids, faceVids).map([&](const auto& _fidAndPidAndVids) {
				const auto& [fid, pid, vids] {_fidAndPidAndVids};
				return extrudeFace(_mesh, pid, fid, vids);
			}).toArray()
		};
		const Id newVid{ _mesh.num_verts() + toId(_newVerts.size()) };
		_newVerts.push_back((newFaceVerts[0][2] + newFaceVerts[1][2] + newFaceVerts[2][2]) / 3);
		if (_clockwise)
		{
			return {
				faceVids[0][0], faceVids[0][1], faceVids[0][2], faceVids[0][3],
				faceVids[2][1], faceVids[2][2], newVid, faceVids[1][2],
			};
		}
		else
		{
			return {
				faceVids[0][0], faceVids[0][1], faceVids[0][2], faceVids[0][3],
				faceVids[1][1], faceVids[1][2], newVid, faceVids[2][2],
			};
		}
	}

	PolyVertIds apply(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude, std::vector<Vec>& _newVerts)
	{
		const Meshing::Mesher::Mesh& mesh{ _mesher.mesh() };
		const cpputils::collections::FixedVector<Id, 3> pids{
			cpputils::range::of(_extrude.parents()).map([&](const Dag::Element& _parent) {
				return _mesher.elementToPid(_parent);
			}).toFixedVector<3>()
		};
		const cpputils::collections::FixedVector<Id, 3> fids{
			cpputils::range::zip(_extrude.parents(), _extrude.fis).map([&](const auto& _parentAndFi) {
				const auto& [parent, fi] {_parentAndFi};
		return Meshing::Utils::fid(mesh, parent, fi);
			}).toFixedVector<3>()
		};
		const Id firstVid{ _extrude.parents().first().vids[_extrude.firstVi] };
		switch (_extrude.source)
		{
			case Dag::Extrude::ESource::Face:
				return applyFaceExtrude(mesh, pids[0], fids[0], firstVid, _newVerts);
			case Dag::Extrude::ESource::Edge:
				return applyEdgeExtrude(mesh, cpputils::range::of(pids).toArray<2>(), cpputils::range::of(fids).toArray<2>(), firstVid, _extrude.clockwise, _newVerts);
			case Dag::Extrude::ESource::Vertex:
				return applyVertexExtrude(mesh, cpputils::range::of(pids).toArray<3>(), cpputils::range::of(fids).toArray<3>(), firstVid, _extrude.clockwise, _newVerts);
			default:
				assert(false);
		}
	}

	PolyVertIds apply(Meshing::Mesher& _mesher, const Dag::Extrude& _extrude)
	{
		std::vector<Vec> newVerts;
		PolyVertIds vids{ apply(_mesher, _extrude, newVerts) };
		Meshing::Utils::addVerts(_mesher, newVerts);
		return vids;
	}

}