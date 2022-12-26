#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <utility>

namespace HMP::Meshing::Utils
{

	Id anyAdjFidInPidByEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid);
	Id adjFidInPidByEidAndFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid);
	Id adjFidInPidByVidAndFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _vid, Id _fid1, Id _fid2);
	Id anyAdjFidInPidByFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid1, Id _fid2);
	Id anyAdjFidInPidByFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid);

	Id sharedEid(const Meshing::Mesher::Mesh& _mesh, Id _pid1, Id _pid2);
	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, Id _eid);
	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVertOffsets, Id _pid);
	EdgeVertIds edgePolyVertOffsets(const Meshing::Mesher::Mesh& _mesh, Id _eid, Id _pid);
	EdgeVertIds edgePolyVertOffsets(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVids, Id _pid);

	bool isEdgeCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, Id _eid);
	bool areVidsCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _vid0, Id _vid1);
	bool isEdgeForward(const FaceVertIds& _vids, Id _vid0, Id _vid1);
	Id nextVidInFid(const FaceVertIds& _vids, Id _vid, bool _backwards = false);

	Real avgFidEdgeLength(const Meshing::Mesher::Mesh& _mesh, Id _fid);

	FaceVertIds pidFidVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _cw = false);
	FaceVertIds pidFidVidsByFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstEid, bool _cw = false);
	FaceVertIds pidFidVidsByFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, bool _cw = false);
	PolyVertIds pidVidsByForwardFidAndFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid);
	PolyVertIds pidVidsByForwardFidAndFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _firstVid);

	FaceVerts verts(const Meshing::Mesher::Mesh& _mesh, const FaceVertIds& _vids);
	PolyVerts verts(const Meshing::Mesher::Mesh& _mesh, const PolyVertIds& _vids);
	Vec midpoint(const Meshing::Mesher::Mesh& _mesh, Id _eid);
	Vec centroid(const PolyVerts& _verts);

	Id closestPolyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid);
	Id closestFaceEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _midpoint);
	Id closestFaceVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _position);

	void addVerts(Mesher& _mesher, const std::vector<Vec>& _verts);
	void addLeafs(Mesher& _mesher, Dag::Node& _root);
	void removeLeafs(Mesher& _mesher, Dag::Node& _root, bool _removeVerts);

	FaceVertIds faceVids(const Dag::Element& _element, I _fi);
	EdgeVertIds edgeVids(const Dag::Element& _element, I _ei);
	I vi(const Dag::Element& _element, Id _vid);
	I fi(const Dag::Element& _element, const FaceVertIds& _vids);
	I ei(const Dag::Element& _element, const EdgeVertIds& _vids);
	FaceVertIds align(const FaceVertIds& _vids, Id _firstVid, bool _reverse = false);
	PolyVertIds align(const PolyVertIds& _vids, Id _firstVid, bool _reverse = false);
	FaceVertIds reverse(const FaceVertIds& _vids);
	PolyVertIds reverse(const PolyVertIds& _vids);
	Id eid(const Mesher::Mesh& _mesh, const Dag::Element& _element, I _ei);
	Id fid(const Mesher::Mesh& _mesh, const Dag::Element& _element, I _fi);
	Vec normal(const FaceVerts& _verts);
	Real avgEdgeLength(const FaceVerts& _verts);

}