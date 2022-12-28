#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <cpputils/range/of.hpp>
#include <utility>

namespace HMP::Meshing::Utils
{

	Id anyAdjFidInPidByEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid);
	Id adjFidInPidByFidAndEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid);
	Id adjFidInPidByVidAndFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _vid, Id _fid1, Id _fid2);
	Id anyAdjFidInPidByFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid1, Id _fid2);
	Id anyAdjFidInPidByFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid);

	Id sharedEid(const Meshing::Mesher::Mesh& _mesh, Id _pid1, Id _pid2);
	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, Id _eid);
	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVertOffsets, Id _pid);
	EdgeVertIds edgeHexVertOffsets(const Meshing::Mesher::Mesh& _mesh, Id _eid, Id _pid);
	EdgeVertIds edgeHexVertOffsets(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVids, Id _pid);

	bool isEdgeCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, Id _eid);
	bool areVidsCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _vid0, Id _vid1);
	bool isEdgeForward(const QuadVertIds& _vids, Id _vid0, Id _vid1);
	Id nextVidInFid(const QuadVertIds& _vids, Id _vid, bool _backwards = false);

	QuadVertIds pidFidVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _cw = false);
	QuadVertIds pidFidVidsByFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstEid, bool _cw = false);
	QuadVertIds pidFidVidsByFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, bool _cw = false);
	HexVertIds pidVidsByForwardFidAndFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid);
	HexVertIds pidVidsByForwardFidAndFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _firstVid);

	template<I TSize>
	std::array<Vec, TSize> verts(const Meshing::Mesher::Mesh& _mesh, const std::array<Id, TSize>& _vids, const std::vector<Vec>& _newVerts = {})
	{
		return cpputils::range::of(_vids).map([&](const Id _vid) {
			return _vid >= _mesh.num_verts() ? _newVerts[toI(_vid - _mesh.num_verts())] : _mesh.vert(_vid);
		}).toArray();
	}

	Vec midpoint(const Meshing::Mesher::Mesh& _mesh, Id _eid);
	Vec centroid(const HexVerts& _verts);

	Id closestPolyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid);
	Id closestFaceEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _midpoint);
	Id closestFaceVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _position);

	QuadVertIds fiVids(const HexVertIds& _vids, I _fi);
	EdgeVertIds eiVids(const HexVertIds& _vids, I _ei);
	I vi(const Dag::Element& _element, Id _vid);
	I fi(const Dag::Element& _element, const QuadVertIds& _vids);
	I ei(const Dag::Element& _element, const EdgeVertIds& _vids);
	QuadVertIds align(const QuadVertIds& _vids, Id _firstVid, bool _reverse = false);
	HexVertIds align(const HexVertIds& _vids, Id _firstVid, bool _reverse = false);
	HexVertIds rotate(const HexVertIds& _vids, Id _forwardFid);
	I oppositeFi(I _fi);
	QuadVertIds reverse(const QuadVertIds& _vids);
	HexVertIds reverse(const HexVertIds& _vids);
	Id eid(const Mesher::Mesh& _mesh, const Dag::Element& _element, I _ei);
	Id fid(const Mesher::Mesh& _mesh, const Dag::Element& _element, I _fi);
	Vec normal(const QuadVerts& _verts);
	Real avgEdgeLength(const QuadVerts& _verts);
	bool isShown(const Dag::Node& _node);
	EdgeVertIds eidVids(const Mesher::Mesh& _mesh, Id _eid);
	QuadVertIds fidVids(const Mesher::Mesh& _mesh, Id _fid);
	HexVertIds pidVids(const Mesher::Mesh& _mesh, Id _pid);
	void addTree(Mesher& _mesher, Dag::Node& _root, const std::vector<Vec>& _newVerts = {});

}