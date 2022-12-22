#include <HMP/Meshing/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <cpputils/range/of.hpp>
#include <stdexcept>

namespace HMP::Meshing::Utils
{

	Id anyAdjFidInPidByFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid)
	{
		return adjFidInPidByEidAndFid(_mesh, _pid, _fid, _mesh.adj_f2e(_fid)[0]);
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
		if (!_mesh.poly_contains_edge(_pid, _eid))
		{
			throw std::logic_error{ "edge not in poly" };
		}
		for (const Id fid : _mesh.adj_e2f(_eid))
		{
			if (_mesh.poly_contains_face(_pid, fid))
			{
				return fid;
			}
		}
		throw std::runtime_error{ "unexpected" };
	}

	Id adjFidInPidByEidAndFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid)
	{
		if (!_mesh.poly_contains_face(_pid, _fid))
		{
			throw std::logic_error{ "face not in poly" };
		}
		if (!_mesh.face_contains_edge(_fid, _eid))
		{
			throw std::logic_error{ "edge not in face" };
		}
		for (const Id fid : _mesh.poly_faces_id(_pid))
		{
			if (fid != _fid && _mesh.face_shared_edge(fid, _fid) == _eid)
			{
				return fid;
			}
		}
		throw std::runtime_error{ "unexpected" };
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
		throw std::logic_error{ "not adjacent" };
	}

	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVertOffsets, Id _pid)
	{
		return {
			_mesh.poly_vert_id(_pid, _edgeVertOffsets[0]),
			_mesh.poly_vert_id(_pid, _edgeVertOffsets[1])
		};
	}

	EdgeVertIds edgeVids(const Meshing::Mesher::Mesh& _mesh, Id _eid)
	{
		return {
			_mesh.edge_vert_id(_eid, 0),
			_mesh.edge_vert_id(_eid, 1)
		};
	}

	EdgeVertIds edgePolyVertOffsets(const Meshing::Mesher::Mesh& _mesh, Id _eid, Id _pid)
	{
		return edgePolyVertOffsets(_mesh, edgeVids(_mesh, _eid), _pid);
	}

	EdgeVertIds edgePolyVertOffsets(const Meshing::Mesher::Mesh& _mesh, const EdgeVertIds& _edgeVids, Id _pid)
	{
		return {
			_mesh.poly_vert_offset(_pid, _edgeVids[0]),
			_mesh.poly_vert_offset(_pid, _edgeVids[1])
		};
	}

	FaceVertIds pidFidVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _cw)
	{
		if (!_mesh.poly_contains_face(_pid, _fid))
		{
			throw std::logic_error{ "face not in poly" };
		}
		std::vector<Id> vids{ _mesh.face_verts_id(_fid) };
		if (_cw == _mesh.poly_face_winding(_pid, _fid))
		{
			std::reverse(vids.begin(), vids.end());
		}
		return cpputils::range::of(vids).toArray<4>();
	}

	FaceVertIds pidFidVidsByFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstEid, bool _cw)
	{
		if (!_mesh.face_contains_edge(_fid, _firstEid))
		{
			throw std::logic_error{ "edge not in face" };
		}
		FaceVertIds vids{ pidFidVids(_mesh, _pid, _fid, _cw) };
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

	FaceVertIds pidFidVidsByFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, bool _cw)
	{
		if (!_mesh.face_contains_vert(_fid, _firstVid))
		{
			throw std::logic_error{ "vert not in face" };
		}
		FaceVertIds vids{ pidFidVids(_mesh, _pid, _fid, _cw) };
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

	bool isEdgeForward(const FaceVertIds& _vids, Id _vid0, Id _vid1)
	{
		const std::size_t index0{ static_cast<std::size_t>(std::distance(_vids.begin(), std::find(_vids.begin(), _vids.end(), _vid0))) };
		return _vids[(index0 + 1) % 4] == _vid1;
	}

	PolyVertIds polyVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid)
	{
		const FaceVertIds forwardFaceVids{ pidFidVidsByFirstEid(_mesh, _pid, _forwardFid, _forwardUpEid, true) };
		FaceVertIds backFaceVids{ pidFidVids(_mesh, _pid, _mesh.poly_face_opposite_to(_pid, _forwardFid), false) };
		while (_mesh.edge_id(forwardFaceVids[0], backFaceVids[0]) == noId)
		{
			std::rotate(backFaceVids.begin(), backFaceVids.begin() + 1, backFaceVids.end());
		}
		PolyVertIds vids;
		std::copy(forwardFaceVids.begin(), forwardFaceVids.end(), vids.begin());
		std::copy(backFaceVids.begin(), backFaceVids.end(), vids.begin() + 4);
		return vids;
	}

	FaceVerts verts(const Meshing::Mesher::Mesh& _mesh, const FaceVertIds& _vids)
	{
		FaceVerts verts;
		for (I i{}; i < 4; i++)
		{
			verts[i] = _mesh.vert(_vids[i]);
		}
		return verts;
	}

	PolyVerts verts(const Meshing::Mesher::Mesh& _mesh, const PolyVertIds& _vids)
	{
		PolyVerts verts;
		for (I i{}; i < 8; i++)
		{
			verts[i] = _mesh.vert(_vids[i]);
		}
		return verts;
	}

	Vec midpoint(const Meshing::Mesher::Mesh& _mesh, Id _eid)
	{
		const std::vector<Vec> verts{ _mesh.edge_verts(_eid) };
		return (verts[0] + verts[1]) / 2;
	}

	Vec centroid(const PolyVerts& _verts)
	{
		Vec centroid{ 0,0,0 };
		for (const Vec& vert : _verts)
		{
			centroid += vert;
		}
		centroid /= 8;
		return centroid;
	}

	PolyVertLoc polyVertLoc(const Vec& _vert, const Vec& _centroid)
	{
		return PolyVertLoc{
			_vert.x() > _centroid.x(),
			_vert.y() > _centroid.y(),
			_vert.z() > _centroid.z()
		};
	}

	PolyVertIds sortVids(const Meshing::Mesher::Mesh& _mesh, const PolyVertIds& _vids)
	{
		PolyVertData<char> indices{};
		for (I i{ 0 }; i < 8; i++)
		{
			indices[sortedPolyVertLocs[i].bits()] = static_cast<char>(i);
		}
		PolyVertIds sortedVids{};
		const PolyVerts verts{ Utils::verts(_mesh, _vids) };
		const Vec centroid(Utils::centroid(verts));
		for (I i{ 0 }; i < 8; i++)
		{
			sortedVids[indices[polyVertLoc(verts[i], centroid).bits()]] = _vids[i];
		}
		return sortedVids;
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
		for (const Id vid : _mesh.face_verts_id(_fid))
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

	void addLeafs(Mesher& _mesher, Dag::Node& _root, bool _clear)
	{
		if (_clear)
		{
			_mesher.clear();
		}

		for (Dag::Node* node : Dag::Utils::descendants(_root))
		{
			if (node->isElement())
			{
				Dag::Element& element{ node->element() };
				bool active{ true };
				for (const Dag::Operation& child : element.children())
				{
					if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
					{
						active = false;
					}
				}
				if (active)
				{
					_mesher.add(element);
				}
			}
		}
	}

	void removeLeafs(Mesher& _mesher, Dag::Node& _root)
	{
		for (Dag::Node* node : Dag::Utils::descendants(_root))
		{
			if (node->isElement())
			{
				Dag::Element& element{ node->element() };
				bool active{ true };
				for (const Dag::Operation& child : element.children())
				{
					if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
					{
						active = false;
					}
				}
				if (active)
				{
					_mesher.remove(element);
				}
			}
		}
	}

}