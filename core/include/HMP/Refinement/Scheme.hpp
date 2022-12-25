#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cstddef>
#include <vector>
#include <array>

namespace HMP::Refinement
{

	class Scheme final: public cpputils::mixins::ReferenceClass
	{

	private:

		const I m_gridSize;
		const std::vector<IVec> m_verts;
		const std::vector<PolyVertData<I>> m_polys;

	public:

		explicit Scheme(I _gridSize, const std::vector<IVec>& _verts, const std::vector<PolyVertData<I>>& _polys);

		I gridSize() const;

		const std::vector<IVec>& verts() const;

		const std::vector<PolyVertData<I>>& polys() const;

		std::vector<PolyVertIds> apply(Meshing::Mesher& _mesher, const PolyVerts& _sourceVerts) const;

	};

}

