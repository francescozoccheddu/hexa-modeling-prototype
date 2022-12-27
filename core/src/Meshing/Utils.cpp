#include <HMP/Meshing/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <cpputils/range/of.hpp>
#include <cinolib/geometry/polygon_utils.h>
#include <cassert>

namespace HMP::Meshing::Utils
{

	Id anyAdjFidInPidByFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid)
	{
		return adjFidInPidByFidAndEid(_mesh, _pid, _fid, _mesh.adj_f2e(_fid)[0]);
	}

	Id adjFidInPidByVidAndFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _vid, Id _fid1, Id _fid2)
	{
		return cpputils::range::of(_mesh.poly_faces_id(_pid)).filter([&](const Id _fid) {
			return _fid != _fid1 && _fid != _fid2
			&& _mesh.faces_are_adjacent(_fid, _fid1)
			&& _mesh.faces_are_adjacent(_fid, _fid2)
			&& _mesh.face_contains_vert(_fid, _vid);
		}).single();
	}

	Id anyAdjFidInPidByFids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid1, Id _fid2)
	{
		return adjFidInPidByVidAndFids(_mesh, _pid, _mesh.edge_vert_id(_mesh.face_shared_edge(_fid1, _fid2), 0), _fid1, _fid2);
	}

	Id anyAdjFidInPidByEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid)
	{
		assert(_mesh.poly_contains_edge(_pid, _eid));
		for (const Id fid : _mesh.adj_e2f(_eid))
		{
			if (_mesh.poly_contains_face(_pid, fid))
			{
				return fid;
			}
		}
		assert(false);
	}

	Id adjFidInPidByFidAndEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid)
	{
		assert(_mesh.poly_contains_face(_pid, _fid));
		assert(_mesh.face_contains_edge(_fid, _eid));
		for (const Id fid : _mesh.poly_faces_id(_pid))
		{
			if (fid != _fid && _mesh.face_shared_edge(fid, _fid) == _eid)
			{
				return fid;
			}
		}
		assert(false);
	}

	Id sharedEid(const Meshing::Mesher::Mesh& _mesh, Id _pid1, Id _pid2)
	{
		for (const Id sharedEid : _mesh.adj_p2e(_pid1))
		{
			for (const Id adjPid : _mesh.adj_e2p(sharedEid))
			{
				if (adjPid == _pid2)
				{
					return sharedEid;
				}
			}
		}
		assert(false);
	}

	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVertOffsets, Id _pid)
	{
		return {
			_mesh.poly_vert_id(_pid, _edgeVertOffsets[0]),
			_mesh.poly_vert_id(_pid, _edgeVertOffsets[1])
		};
	}

	Id nextVidInFid(const QuadVertIds& _vids, Id _vid, bool _backwards)
	{
		const int index{ static_cast<int>(std::distance(_vids.begin(), std::find(_vids.begin(), _vids.end(), _vid))) };
		return _vids[(index + (_backwards ? -1 : 1)) % 4];
	}

	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, Id _eid)
	{
		return {
			_mesh.edge_vert_id(_eid, 0),
			_mesh.edge_vert_id(_eid, 1)
		};
	}

	EdgeVertIds edgeHexVertOffsets(const Meshing::Mesher::Mesh& _mesh, Id _eid, Id _pid)
	{
		return edgeHexVertOffsets(_mesh, edgeVids(_mesh, _eid), _pid);
	}

	EdgeVertIds edgeHexVertOffsets(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVids, Id _pid)
	{
		return {
			_mesh.poly_vert_offset(_pid, _edgeVids[0]),
			_mesh.poly_vert_offset(_pid, _edgeVids[1])
		};
	}

	QuadVertIds pidFidVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _cw)
	{
		assert(_mesh.poly_contains_face(_pid, _fid));
		QuadVertIds vids{ fidVids(_mesh, _fid) };
		if (_cw == _mesh.poly_face_winding(_pid, _fid))
		{
			std::reverse(vids.begin(), vids.end());
		}
		return cpputils::range::of(vids).toArray<4>();
	}

	QuadVertIds pidFidVidsByFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstEid, bool _cw)
	{
		assert(_mesh.face_contains_edge(_fid, _firstEid));
		QuadVertIds vids{ pidFidVids(_mesh, _pid, _fid, _cw) };
		while (_mesh.edge_id(vids[0], vids[1]) != _firstEid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.end());
		}
		return vids;
	}

	Real avgFidEdgeLength(const Meshing::Mesher::Mesh& _mesh, Id _fid)
	{
		return cpputils::range::of(_mesh.adj_f2e(_fid)).map([&](Id _eid) { return _mesh.edge_length(_eid);}).avg();
	}

	QuadVertIds pidFidVidsByFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, bool _cw)
	{
		assert(_mesh.face_contains_vert(_fid, _firstVid));
		QuadVertIds vids{ pidFidVids(_mesh, _pid, _fid, _cw) };
		while (vids[0] != _firstVid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.end());
		}
		return vids;
	}

	bool isEdgeCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, Id _eid)
	{
		return areVidsCW(_mesh, _pid, _fid, _firstVid, _mesh.vert_opposite_to(_eid, _firstVid));
	}

	bool areVidsCW(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _vid0, Id _vid1)
	{
		return !isEdgeForward(pidFidVids(_mesh, _pid, _fid), _vid0, _vid1);
	}

	bool isEdgeForward(const QuadVertIds& _vids, Id _vid0, Id _vid1)
	{
		const I index0{ static_cast<I>(std::distance(_vids.begin(), std::find(_vids.begin(), _vids.end(), _vid0))) };
		return _vids[(index0 + 1) % 4] == _vid1;
	}

	HexVertIds pidVidsByForwardFidAndFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid)
	{
		const QuadVertIds forwardFaceVids{ pidFidVidsByFirstEid(_mesh, _pid, _forwardFid, _forwardUpEid, true) };
		QuadVertIds backFaceVids{ pidFidVids(_mesh, _pid, _mesh.poly_face_opposite_to(_pid, _forwardFid), false) };
		while (_mesh.edge_id(forwardFaceVids[0], backFaceVids[0]) == noId)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}
		HexVertIds vids;
		std::copy(forwardFaceVids.begin(), forwardFaceVids.end(), vids.begin());
		std::copy(backFaceVids.begin(), backFaceVids.end(), vids.begin() + 4);
		return vids;
	}

	HexVertIds pidVidsByForwardFidAndFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _firstVid)
	{
		QuadVertIds forwardFaceVids{ pidFidVidsByFirstVid(_mesh, _pid, _forwardFid, _firstVid) };
		QuadVertIds backFaceVids{ pidFidVids(_mesh, _pid, _mesh.poly_face_opposite_to(_pid, _forwardFid), true) };
		while (_mesh.edge_id(forwardFaceVids[0], backFaceVids[0]) == noId)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}
		HexVertIds vids;
		std::copy(forwardFaceVids.begin(), forwardFaceVids.end(), vids.begin());
		std::copy(backFaceVids.begin(), backFaceVids.end(), vids.begin() + 4);
		return vids;
	}

	Vec midpoint(const Meshing::Mesher::Mesh& _mesh, Id _eid)
	{
		const std::vector<Vec> verts{ _mesh.edge_verts(_eid) };
		return (verts[0] + verts[1]) / 2;
	}

	Vec centroid(const HexVerts& _verts)
	{
		Vec centroid{ 0,0,0 };
		for (const Vec& vert : _verts)
		{
			centroid += vert;
		}
		centroid /= 8;
		return centroid;
	}

	Id closestPolyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestFid{};
		for (const Id fid : _mesh.poly_faces_id(_pid))
		{
			const Real dist{ _centroid.dist(_mesh.face_centroid(fid)) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestFid = fid;
			}
		}
		return closestFid;
	}

	Id closestFaceVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _position)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestVid{};
		for (const Id vid : fidVids(_mesh, _fid))
		{
			const Real dist{ _position.dist(_mesh.vert(vid)) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestVid = vid;
			}
		}
		return closestVid;
	}

	Id closestFaceEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _midpoint)
	{
		Real closestDist{ cinolib::inf_double };
		Id closestEid{};
		for (Id edgeOffset{ 0 }; edgeOffset < 4; edgeOffset++)
		{
			const Id eid{ _mesh.face_edge_id(_fid, edgeOffset) };
			const Vec midpoint{ Utils::midpoint(_mesh, eid) };
			const Real dist{ _midpoint.dist(midpoint) };
			if (dist < closestDist)
			{
				closestDist = dist;
				closestEid = eid;
			}
		}
		return closestEid;
	}

	QuadVertIds faceVids(const Dag::Element& _element, I _fi)
	{
		const HexVertIds vids{ _element.vids };
		switch (_fi)
		{
			case 0:
				return { vids[0], vids[3], vids[2], vids[1] };
			case 1:
				return { vids[4], vids[5], vids[6], vids[7] };
			case 2:
				return { vids[1], vids[2], vids[6], vids[5] };
			case 3:
				return { vids[0], vids[4], vids[7], vids[3] };
			case 4:
				return { vids[0], vids[1], vids[5], vids[4] };
			case 5:
				return { vids[3], vids[7], vids[6], vids[2] };
			default:
				assert(false);
		}
	}

	QuadVertIds align(const QuadVertIds& _vids, Id _firstVid, bool _reverse)
	{
		QuadVertIds vids{ _vids };
		while (vids[0] != _firstVid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.end());
		}
		if (_reverse)
		{
			vids = reverse(vids);
		}
		return vids;
	}

	HexVertIds align(const HexVertIds& _vids, Id _firstVid, bool _reverse)
	{
		HexVertIds vids{ _vids };
		while (vids[0] != _firstVid)
		{
			std::rotate(vids.begin(), vids.begin() + 1, vids.begin() + 4);
			std::rotate(vids.begin() + 4, vids.begin() + 4 + 1, vids.end());
		}
		if (_reverse)
		{
			vids = reverse(vids);
		}
		return vids;
	}

	QuadVertIds reverse(const QuadVertIds& _vids)
	{
		return {
			_vids[0],
			_vids[3],
			_vids[2],
			_vids[1]
		};
	}

	HexVertIds reverse(const HexVertIds& _vids)
	{
		return {
			_vids[0],
			_vids[3],
			_vids[2],
			_vids[1],
			_vids[4],
			_vids[7],
			_vids[6],
			_vids[5]
		};
	}

	I vi(const Dag::Element& _element, Id _vid)
	{
		for (I vi{}; vi < 8; vi++)
		{
			if (_element.vids[vi] == _vid)
			{
				return vi;
			}
		}
		assert(false);
	}

	I fi(const Dag::Element& _element, const QuadVertIds& _vids)
	{
		QuadVertIds qVids{ _vids };
		std::sort(qVids.begin(), qVids.end());
		for (I fi{}; fi < 6; fi++)
		{
			QuadVertIds vids{ faceVids(_element, fi) };
			std::sort(vids.begin(), vids.end());
			if (vids == qVids)
			{
				return fi;
			}
		}
		assert(false);
	}

	EdgeVertIds edgeVids(const Dag::Element& _element, I _ei)
	{
		const HexVertIds vids{ _element.vids };
		switch (_ei)
		{
			case 0:
				return { vids[0], vids[1] };
			case 1:
				return { vids[1], vids[2] };
			case 2:
				return { vids[2], vids[3] };
			case 3:
				return { vids[3], vids[0] };
			case 4:
				return { vids[4], vids[5] };
			case 5:
				return { vids[5], vids[6] };
			case 6:
				return { vids[6], vids[7] };
			case 7:
				return { vids[7], vids[4] };
			case 8:
				return { vids[0], vids[4] };
			case 9:
				return { vids[1], vids[5] };
			case 10:
				return { vids[2], vids[6] };
			case 11:
				return { vids[3], vids[7] };
			default:
				assert(false);
		}
	}

	I ei(const Dag::Element& _element, const EdgeVertIds& _vids)
	{
		EdgeVertIds qVids{ _vids };
		std::sort(qVids.begin(), qVids.end());
		for (I ei{}; ei < 12; ei++)
		{
			EdgeVertIds vids{ edgeVids(_element, ei) };
			std::sort(vids.begin(), vids.end());
			if (vids == qVids)
			{
				return ei;
			}
		}
		assert(false);
	}

	Id eid(const Mesher::Mesh& _mesh, const Dag::Element& _element, I _ei)
	{
		const int eid{ _mesh.edge_id(cpputils::range::of(edgeVids(_element, _ei)).toVector()) };
		assert(eid != -1);
		return static_cast<Id>(eid);
	}

	Id fid(const Mesher::Mesh& _mesh, const Dag::Element& _element, I _fi)
	{
		const int fid{ _mesh.face_id(cpputils::range::of(faceVids(_element, _fi)).toVector()) };
		assert(fid != -1);
		return static_cast<Id>(fid);
	}

	Vec normal(const QuadVerts& _verts)
	{
		return cinolib::polygon_normal(cpputils::range::of(_verts).toVector());
	}

	Real avgEdgeLength(const QuadVerts& _verts)
	{
		Real sum{};
		for (I i{}; i < 4; i++)
		{
			sum += _verts[i].dist(_verts[(i + 1) % 4]);
		}
		return sum / 4.0;
	}

	bool isShown(const Dag::Node& _node)
	{
		return _node.isElement() && _node.element().children.filter([&](const Dag::Operation& _child) {
			return _child.primitive != Dag::Operation::EPrimitive::Extrude;
		}).empty();
	}

	EdgeVertIds eidVids(const Mesher::Mesh& _mesh, Id _eid)
	{
		return cpputils::range::of(_mesh.adj_e2v(_eid)).toArray<2>();
	}

	QuadVertIds fidVids(const Mesher::Mesh& _mesh, Id _fid)
	{
		return cpputils::range::of(_mesh.adj_f2v(_fid)).toArray<4>();
	}

	HexVertIds pidVids(const Mesher::Mesh& _mesh, Id _pid)
	{
		return cpputils::range::of(_mesh.adj_p2v(_pid)).toArray<8>();
	}

	void addTree(Mesher& _mesher, Dag::Node& _root, const std::vector<Vec>& _newVerts)
	{
		const std::vector<Dag::Element*> elements{
			cpputils::range::of(Dag::Utils::descendants(_root))
			.filter([&](const Dag::Node* _node) { return _node->isElement(); })
			.cast<Dag::Element*>()
			.toVector()
		};
		_mesher.add(elements, _newVerts);
		for (Dag::Element* element : elements)
		{
			if (!isShown(*element))
			{
				_mesher.show(*element, false);
			}
		}
	}

}