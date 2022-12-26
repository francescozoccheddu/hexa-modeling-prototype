#include <HMP/Refinement/Scheme.hpp>

#include <cinolib/geometry/lerp.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <limits>

namespace HMP::Refinement
{

	Scheme::Scheme(I _gridSize, const std::vector<IVec>& _verts, const std::vector<PolyVertData<I>>& _polys)
		: m_gridSize{ _gridSize }, m_verts{ _verts }, m_polys{ _polys }
	{}

	I Scheme::gridSize() const
	{
		return m_gridSize;
	}

	const std::vector<IVec>& Scheme::verts() const
	{
		return m_verts;
	}

	const std::vector<PolyVertData<I>>& Scheme::polys() const
	{
		return m_polys;
	}

	Id getOrAddVert(const Meshing::Mesher::Mesh& _mesh, const Vec& _vert, std::vector<Vec>& _newVerts)
	{
		Real minDist{ std::numeric_limits<Real>::infinity() };
		Id minVid{ noId };
		for (Id vid{}; vid < _mesh.num_verts(); vid++)
		{
			const Real dist{ _mesh.vert(vid).dist(_vert) };
			if (dist < minDist)
			{
				minDist = dist;
				minVid = vid;
			}
		}
		static constexpr Real eps{ 1e-9 };
		if (minDist < eps)
		{
			return minVid;
		}
		else
		{
			const Id vid{ _mesh.num_verts() + toId(_newVerts.size()) };
			_newVerts.push_back(_vert);
			return vid;
		}
	}

	std::vector<PolyVertIds> Scheme::apply(const Meshing::Mesher::Mesh& _mesh, const PolyVerts& _sourceVerts, std::vector<Vec>& _newVerts) const
	{
		std::vector<Id> vidsPool;
		vidsPool.reserve(m_verts.size());
		const PolyVerts sourceVerts{
			_sourceVerts[0],
			_sourceVerts[1],
			_sourceVerts[3],
			_sourceVerts[2],
			_sourceVerts[4],
			_sourceVerts[5],
			_sourceVerts[7],
			_sourceVerts[6]
		};
		for (const IVec& ivert : m_verts)
		{
			const Vec progress{ ivert.cast<Real>() / static_cast<Real>(m_gridSize) };
			const Vec vert{ cinolib::lerp3(sourceVerts, progress) };
			vidsPool.push_back(getOrAddVert(_mesh, vert, _newVerts));
		}
		std::vector<PolyVertIds> polys;
		polys.reserve(m_polys.size());
		for (const PolyVertData<I>& polyVis : m_polys)
		{
			PolyVertIds polyVids;
			for (I i{}; i < 8; i++)
			{
				polyVids[i] = vidsPool[polyVis[i]];
			}
			polys.push_back(polyVids);
		}
		return polys;
	}

	std::vector<PolyVertIds> Scheme::apply(Meshing::Mesher& _mesher, const PolyVerts& _sourceVerts) const
	{
		std::vector<Vec> newVerts;
		const std::vector<PolyVertIds> polys{ apply(_mesher.mesh(), _sourceVerts, newVerts) };
		Meshing::Utils::addVerts(_mesher, newVerts);
		return polys;
	}

}