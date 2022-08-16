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

		constexpr PolyVertLoc(bool _x, bool _y, bool _z) : m_bits{ _x << c_bitX | _y << c_bitY | _z << c_bitZ }
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
		PolyVertLoc{true,false,false},
		PolyVertLoc{true,false,true},
		PolyVertLoc{false,false,true},
		PolyVertLoc{false,true,false},
		PolyVertLoc{true,true,false},
		PolyVertLoc{true,true,true},
		PolyVertLoc{false,true,true}
	};

	Id rotateEid(const Meshing::Mesher::Mesh& _mesh, Id _fid, Id _eid, int _rotation);

	Id anyFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _eid);
	Id adjacentFid(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _eid);

	FaceVertIds faceVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, bool _winding = false);
	FaceVertIds faceVids(const Meshing::Mesher::Mesh& _mesh, Id _pid, Id _fid, Id _upEid, bool _winding = false);
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