#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <utility>

namespace HMP::Meshing::Utils
{

	class PolyVertLoc final
	{

	private:

		char m_bits;

	public:

		static constexpr int c_bitX{ 0 }, c_bitY{ 1 }, c_bitZ{ 2 };
		static constexpr int c_maskX{ 1 << c_bitX }, c_maskY{ 1 << c_bitY }, c_maskZ{ 1 << c_bitZ };

		constexpr PolyVertLoc(bool _x, bool _y, bool _z): m_bits{ static_cast<char>(_x << c_bitX | _y << c_bitY | _z << c_bitZ) }
		{}

		constexpr char bits() const
		{
			return m_bits;
		}

		constexpr bool x() const
		{
			return m_bits & c_maskX;
		}

		constexpr bool y() const
		{
			return m_bits & c_maskY;
		}

		constexpr bool z() const
		{
			return m_bits & c_maskZ;
		}

	};

	constexpr PolyVertData<PolyVertLoc> sortedPolyVertLocs{
		PolyVertLoc{false,false,false},
		PolyVertLoc{false,false,true},
		PolyVertLoc{true,false,true},
		PolyVertLoc{true,false,false},
		PolyVertLoc{false,true,false},
		PolyVertLoc{false,true,true},
		PolyVertLoc{true,true,true},
		PolyVertLoc{true,true,false},
	};

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

	FaceVertIds pidFidVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _winding = false);
	FaceVertIds pidFidVidsByFirstEid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstEid, bool _winding = false);
	FaceVertIds pidFidVidsByFirstVid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _firstVid, bool _winding = false);
	PolyVertIds polyVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _forwardFid, Id _forwardUpEid);

	FaceVerts verts(const Meshing::Mesher::Mesh& _mesh, const FaceVertIds& _vids);
	PolyVerts verts(const Meshing::Mesher::Mesh& _mesh, const PolyVertIds& _vids);
	Vec midpoint(const Meshing::Mesher::Mesh& _mesh, Id _eid);
	Vec centroid(const PolyVerts& _verts);

	PolyVertLoc polyVertLoc(const Vec& _vert, const Vec& _centroid);
	PolyVertIds sortVids(const Meshing::Mesher::Mesh& _mesh, const PolyVertIds& _vids);

	Id closestPolyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, const Vec& _centroid);
	Id closestFaceEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _midpoint);
	Id closestFaceVid(const Meshing::Mesher::Mesh& _mesh, Id _fid, const Vec& _position);

	void addLeafs(Mesher& _mesher, Dag::Node& _root, bool _clear = true);
	void removeLeafs(Mesher& _mesher, Dag::Node& _root);

}