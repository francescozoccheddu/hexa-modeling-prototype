#include <HMP/Refinement/Utils.hpp>

#include <HMP/Dag/Utils.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/enumerate.hpp>
#include <cpputils/collections/FixedVector.hpp>
#include <cinolib/geometry/lerp.hpp>
#include <cassert>
#include <vector>
#include <cstddef>
#include <algorithm>
#include <unordered_set>
#include <limits>

namespace HMP::Refinement::Utils
{

	Dag::Refine& prepare(I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth)
	{
		assert(_depth >= 1 && _depth <= 3);
		Dag::Refine& refine{ *new Dag::Refine{} };
		refine.scheme = _scheme;
		refine.forwardFi = _forwardFi;
		refine.firstVi = _firstVi;
		const Refinement::Scheme& scheme{ Refinement::schemes.at(_scheme) };
		for (I i{ 0 }; i < scheme.polys.size(); i++)
		{
			Dag::Element& child{ *new Dag::Element{} };
			if (_depth > 1)
			{
				child.children.attach(prepare(_forwardFi, _firstVi, _scheme, _depth - 1));
			}
			refine.children.attach(child);
		}
		return refine;
	}

	IVec schemeOrigin(I _vi, I _gridSize)
	{
		const I s{ _gridSize };
		switch (_vi)
		{
			case 0:
				return { 0,0,0 };
			case 1:
				return { s,0,0 };
			case 2:
				return { s,s,0 };
			case 3:
				return { 0,s,0 };
			case 4:
				return { 0,0,s };
			case 5:
				return { s,0,s };
			case 6:
				return { s,s,s };
			case 7:
				return { 0,s,s };
			default:
				assert(false);
		}
	}

	struct BasisVec final
	{
		bool dim;
		bool positive;

		constexpr BasisVec operator-() const
		{
			return { dim, !positive };
		}

	};

	I index(const QuadVertIs& _elements, I _element)
	{
		return static_cast<I>(std::distance(_elements.begin(), std::find(_elements.begin(), _elements.end(), _element)));
	}

	std::array<BasisVec, 2> rotateBasisCCW(I _count, const BasisVec& _right, const BasisVec& _up)
	{
		switch (_count % 4)
		{
			case 0:
				return { _right, _up };
			case 1:
				return { _up, -_right };
			case 2:
				return { -_right, -_up };
			case 3:
				return { -_up, _right };
			default:
				assert(false);
		}
	}

	static constexpr HexFaceData<QuadVertIs> faceVis{ {
		{ 0,1,2,3 },
		{ 4,7,6,5 },
		{ 1,5,6,2 },
		{ 0,3,7,4 },
		{ 0,4,5,1 },
		{ 3,2,6,7 }
	} };

	std::array<BasisVec, 2> schemeBasis(I _fi, I _vi)
	{
		static constexpr Id x{ 0 }, y{ 1 }, z{ 2 };
		static constexpr HexFaceData<std::pair<I, I>> dims{ {
			{x,y},
			{y,x},
			{z,y},
			{y,z},
			{z,x},
			{x,z}
		} };
		const auto& [rightDim, upDim] { dims[_fi] };
		return rotateBasisCCW(index(faceVis[_fi], _vi), { rightDim > upDim, true }, { upDim > rightDim, true });
	}

	I rotateFiCW(I _fi, I _forwardFi)
	{
		static constexpr HexFaceData<HexFaceIs> rotFis{ {
			{0,1,4,5,3,2},
			{0,1,5,4,2,3},
			{5,4,2,3,0,1},
			{4,5,2,3,1,0},
			{2,3,1,0,4,5},
			{3,2,0,1,4,5}
		} };
		return rotFis[_forwardFi][_fi];
	}

	I rotateFiCCW(I _fi, I _forwardFi)
	{
		static constexpr HexFaceData<HexFaceIs> rotFis{ {
			{0,1,5,4,2,3},
			{0,1,4,5,3,2},
			{4,5,2,3,1,0},
			{5,4,2,3,0,1},
			{3,2,0,1,4,5},
			{2,3,1,0,4,5}
		} };
		return rotFis[_forwardFi][_fi];
	}

	I relativeFi(I _fi, I _forwardFi, I _vi)
	{
		static constexpr HexFaceData<HexFaceIs> invFis{ {
			{0,1,2,3,4,5},
			{1,0,5,4,3,2},
			{3,2,0,1,4,5},
			{4,5,1,0,3,2},
			{3,2,5,4,0,1},
			{4,5,2,3,1,0}
		} };
		I count{ index(faceVis[_forwardFi], _vi) };
		while (count-- > 0)
		{
			_fi = rotateFiCW(_fi, _forwardFi);
		}
		return invFis[_forwardFi][_fi];
	}

	void apply(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		const Scheme& scheme{ schemes.at(_refine.scheme) };
		std::vector<Id> schemeVids(scheme.verts.size(), noId);
		Dag::Element& parent{ _refine.parents.single() };
		const HexVertIds parentVids{ Meshing::Utils::align(Meshing::Utils::rotate(parent.vids, _refine.forwardFi), parent.vids[_refine.firstVi]) };
		// weld adjacencies
		{
			const IVec origin{ schemeOrigin(_refine.firstVi, scheme.gridSize) };
			const std::array<BasisVec, 2> basis{ schemeBasis(_refine.forwardFi, _refine.firstVi) };
			// ################# TEMP IMPL ##################
			std::unordered_set<Id> vidsSearchPool{}; // TEMP IMPL
			// ##############################################
			for (const Id adjPid : _mesher.mesh().adj_p2p(parent.pid))
			{
				const Dag::Refine* adjRefine{
					_mesher
					.element(adjPid)
					.children
					.filter([&](const Dag::Operation& _op) { return _op.primitive == Dag::Operation::EPrimitive::Refine; })
					.address()
					.cast<const Dag::Refine*>()
					.single(nullptr)
				};
				if (!adjRefine)
				{
					continue;
				}
				const Scheme& adjScheme{ schemes.at(adjRefine->scheme) };
				if (adjScheme.gridSize == scheme.gridSize)
				{
					const Id fid{ static_cast<Id>(_mesher.mesh().poly_shared_face(parent.pid, adjPid)) };
					const QuadVertIds fidVids{ Meshing::Utils::fidVids(_mesher.mesh(), fid) };
					const I fi{ Meshing::Utils::fi(parent.vids, fidVids) };
					const I adjFi{ Meshing::Utils::fi(adjRefine->parents.single().vids, fidVids) };
					const I schemeFi{ relativeFi(fi, _refine.forwardFi, _refine.firstVi) };
					const I adjSchemeFi{ relativeFi(adjFi, adjRefine->forwardFi, adjRefine->firstVi) };
					const I count{ scheme.facesSurfVisIs[schemeFi].size() / 4 };
					const I adjCount{ adjScheme.facesSurfVisIs[adjSchemeFi].size() / 4 };
					// ################# TEMP IMPL ##################
					vidsSearchPool.insert(adjRefine->surfVids.begin(), adjRefine->surfVids.end()); // TEMP IMPL
					// ##############################################
				}
			}
			// ################# TEMP IMPL ##################
			{  // TEMP IMPL
				const Id numVerts{ _mesher.mesh().num_verts() }; // TEMP IMPL
				const HexVerts parentVerts{ Meshing::Utils::verts(_mesher.mesh(), parentVids) }; // TEMP IMPL
				const HexVerts lerpVerts{ // TEMP IMPL
					parentVerts[0], // TEMP IMPL
					parentVerts[1], // TEMP IMPL
					parentVerts[3], // TEMP IMPL
					parentVerts[2], // TEMP IMPL
					parentVerts[4], // TEMP IMPL
					parentVerts[5], // TEMP IMPL
					parentVerts[7], // TEMP IMPL
					parentVerts[6] // TEMP IMPL
				}; // TEMP IMPL
				const Real eps{ 1e-9 }; // TEMP IMPL
				for (const auto& [vid, ivert] : cpputils::range::zip(schemeVids, scheme.verts)) // TEMP IMPL
				{ // TEMP IMPL
					const Vec progress{ ivert.cast<Real>() / static_cast<Real>(scheme.gridSize) }; // TEMP IMPL
					const Vec vert{ cinolib::lerp3(lerpVerts, progress) }; // TEMP IMPL
					Real minDist{ std::numeric_limits<Real>::infinity() }; // TEMP IMPL
					for (const Id candVid : vidsSearchPool) // TEMP IMPL
					{ // TEMP IMPL
						const Real dist{ _mesher.mesh().vert(candVid).dist(vert) }; // TEMP IMPL
						if (dist < minDist && dist < eps) // TEMP IMPL
						{ // TEMP IMPL
							minDist = dist; // TEMP IMPL
							vid = candVid; // TEMP IMPL
						} // TEMP IMPL
					} // TEMP IMPL
				} // TEMP IMPL
			} // TEMP IMPL
			// ##############################################
		}
		// weld corners
		for (const I cornerVi : scheme.cornerVis)
		{
			const IVec& corner{ scheme.verts[cornerVi] };
			schemeVids[cornerVi] = parentVids[scheme.cornerVi(corner)];
		}
		// create missing verts
		std::vector<Vec> newVerts;
		{
			const Id numVerts{ _mesher.mesh().num_verts() };
			const HexVerts parentVerts{ Meshing::Utils::verts(_mesher.mesh(), parentVids) };
			const HexVerts lerpVerts{
				parentVerts[0],
				parentVerts[1],
				parentVerts[3],
				parentVerts[2],
				parentVerts[4],
				parentVerts[5],
				parentVerts[7],
				parentVerts[6]
			};
			for (const auto& [vid, ivert] : cpputils::range::zip(schemeVids, scheme.verts))
			{
				if (vid == noId)
				{
					vid = numVerts + toId(newVerts.size());
					const Vec progress{ ivert.cast<Real>() / static_cast<Real>(scheme.gridSize) };
					newVerts.push_back(cinolib::lerp3(lerpVerts, progress));
				}
			}
		}
		// create elements
		{
			const std::vector<Dag::Element*> elements{
				_refine.children.zip(scheme.polys).map([&](const auto& _elAndVis) {
					const auto& [el, vis] { _elAndVis };
					for (const auto& [vid, vi] : cpputils::range::zip(el.vids, vis))
					{
						vid = schemeVids[vi];
					}
					return &el;
			}).toVector() };
			_refine.surfVids = cpputils::range::of(scheme.surfVis).map([&](const I _vi) { return schemeVids[_vi]; }).toVector();
			_mesher.show(parent, false);
			_mesher.add(elements, newVerts);
		}
	}

	void applyRecursive(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		apply(_mesher, _refine);
		for (Dag::Element& child : _refine.children)
		{
			for (Dag::Operation& operation : child.children)
			{
				if (operation.primitive == Dag::Operation::EPrimitive::Refine)
				{
					applyRecursive(_mesher, static_cast<Dag::Refine&>(operation));
				}
			}
		}
	}

}