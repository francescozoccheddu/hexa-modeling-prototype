#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <vector>

namespace HMP::Refinement
{

	class Scheme final: public cpputils::mixins::ReferenceClass
	{

	private:

		const I m_gridSize;
		const std::vector<IVec> m_verts;
		const std::vector<HexVertData<I>> m_polys;

	public:

		explicit Scheme(I _gridSize, const std::vector<IVec>& _verts, const std::vector<HexVertData<I>>& _polys);

		I gridSize() const;

		const std::vector<IVec>& verts() const;

		const std::vector<HexVertData<I>>& polys() const;

		std::vector<HexVertIds> apply(const Meshing::Mesher& _mesher, const HexVerts& _sourceVerts, std::vector<Vec>& _newVerts) const;

	};

}

