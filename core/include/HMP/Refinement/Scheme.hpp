#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <vector>
#include <map>

namespace HMP::Refinement
{

	class Scheme final: public cpputils::mixins::ReferenceClass
	{

	private:

		static bool compareIVec2(const IVec2& _a, const IVec2& _b);

	public:

		using FaceSurfVisMap = std::map<IVec2, I, decltype(&compareIVec2)>;

	private:

		const I m_gridSize;
		const std::vector<IVec> m_verts;
		const std::vector<HexVertData<I>> m_polys;
		const std::vector<I> m_surfVis;
		const std::vector<I> m_cornerVis;
		const HexFaceData<FaceSurfVisMap> m_facesSurfVisIs;

		std::vector<I> findSurfVis() const;
		std::vector<I> findCornerVis() const;
		FaceSurfVisMap findFacesSurfVisIs(Id _dim, bool _polarity) const;
		HexFaceData<FaceSurfVisMap> findFacesSurfVisIs() const;

	public:

		explicit Scheme(I _gridSize, const std::vector<IVec>& _verts, const std::vector<HexVertData<I>>& _polys);

		I gridSize() const;

		bool isMin(I _comp) const;

		bool isMax(I _comp) const;

		bool isExtreme(I _comp) const;

		bool isOnSurf(const IVec& _vert) const;

		bool isCorner(const IVec& _vert) const;

		const std::vector<IVec>& verts() const;

		const std::vector<HexVertData<I>>& polys() const;

		const std::vector<I>& surfVis() const;

		const std::vector<I>& cornerVis() const;

		const HexFaceData<FaceSurfVisMap>& facesSurfVisIs() const;

	};

}
