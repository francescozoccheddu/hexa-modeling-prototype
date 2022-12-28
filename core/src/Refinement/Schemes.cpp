#include <HMP/Refinement/Schemes.hpp>

namespace HMP::Refinement
{

	namespace Schemes
	{

		const Scheme test{
			7,
			std::vector<IVec>{
				IVec{1, 6, 3},
				IVec{2, 6, 3},
				IVec{1, 7, 3},
				IVec{2, 7, 3},
				IVec{1, 6, 4},
				IVec{2, 6, 4},
				IVec{1, 7, 4},
				IVec{2, 7, 4},
				IVec{1, 6, 1},
				IVec{2, 6, 1},
				IVec{1, 7, 1},
				IVec{2, 7, 1},
				IVec{1, 6, 2},
				IVec{2, 6, 2},
				IVec{1, 7, 2},
				IVec{2, 7, 2},
				IVec{3, 6, 1},
				IVec{4, 6, 1},
				IVec{3, 7, 1},
				IVec{4, 7, 1},
				IVec{3, 6, 2},
				IVec{4, 6, 2},
				IVec{3, 7, 2},
				IVec{4, 7, 2},
				IVec{3, 6, 3},
				IVec{4, 6, 3},
				IVec{3, 7, 3},
				IVec{4, 7, 3},
				IVec{3, 6, 4},
				IVec{4, 6, 4},
				IVec{3, 7, 4},
				IVec{4, 7, 4},
				IVec{3, 6, 5},
				IVec{4, 6, 5},
				IVec{3, 7, 5},
				IVec{4, 7, 5},
				IVec{3, 6, 6},
				IVec{4, 6, 6},
				IVec{3, 7, 6},
				IVec{4, 7, 6},
				IVec{3, 0, 1},
				IVec{4, 0, 1},
				IVec{3, 1, 1},
				IVec{4, 1, 1},
				IVec{3, 0, 2},
				IVec{4, 0, 2},
				IVec{3, 1, 2},
				IVec{4, 1, 2},
				IVec{1, 0, 1},
				IVec{2, 0, 1},
				IVec{1, 1, 1},
				IVec{2, 1, 1},
				IVec{1, 0, 2},
				IVec{2, 0, 2},
				IVec{1, 1, 2},
				IVec{2, 1, 2},
				IVec{1, 0, 3},
				IVec{2, 0, 3},
				IVec{1, 1, 3},
				IVec{2, 1, 3},
				IVec{1, 0, 4},
				IVec{2, 0, 4},
				IVec{1, 1, 4},
				IVec{2, 1, 4},
				IVec{1, 0, 5},
				IVec{2, 0, 5},
				IVec{1, 1, 5},
				IVec{2, 1, 5},
				IVec{1, 0, 6},
				IVec{2, 0, 6},
				IVec{1, 1, 6},
				IVec{2, 1, 6},
				IVec{1, 2, 6},
				IVec{2, 2, 6},
				IVec{1, 1, 7},
				IVec{2, 1, 7},
				IVec{1, 2, 7},
				IVec{2, 2, 7},
				IVec{6, 1, 5},
				IVec{7, 1, 5},
				IVec{6, 2, 5},
				IVec{7, 2, 5},
				IVec{6, 1, 6},
				IVec{7, 1, 6},
				IVec{6, 2, 6},
				IVec{7, 2, 6},
				IVec{6, 3, 5},
				IVec{7, 3, 5},
				IVec{6, 4, 5},
				IVec{7, 4, 5},
				IVec{6, 3, 6},
				IVec{7, 3, 6},
				IVec{6, 4, 6},
				IVec{7, 4, 6},
				IVec{0, 3, 5},
				IVec{1, 3, 5},
				IVec{0, 4, 5},
				IVec{1, 4, 5},
				IVec{0, 3, 6},
				IVec{1, 3, 6},
				IVec{0, 4, 6},
				IVec{1, 4, 6},
				IVec{0, 1, 5},
				IVec{0, 2, 5},
				IVec{1, 2, 5},
				IVec{0, 1, 6},
				IVec{0, 2, 6},
				IVec{0, 5, 5},
				IVec{1, 5, 5},
				IVec{0, 6, 5},
				IVec{1, 6, 5},
				IVec{0, 5, 6},
				IVec{1, 5, 6},
				IVec{0, 6, 6},
				IVec{1, 6, 6},
				IVec{6, 1, 1},
				IVec{6, 6, 1},
				IVec{6, 6, 6},
			},
			std::vector<HexVertData<I>>{
				HexVertData<I>{0, 2, 3, 1, 4, 6, 7, 5, },
				HexVertData<I>{8, 10, 11, 9, 12, 14, 15, 13, },
				HexVertData<I>{16, 18, 19, 17, 20, 22, 23, 21, },
				HexVertData<I>{24, 26, 27, 25, 28, 30, 31, 29, },
				HexVertData<I>{32, 34, 35, 33, 36, 38, 39, 37, },
				HexVertData<I>{40, 42, 43, 41, 44, 46, 47, 45, },
				HexVertData<I>{48, 50, 51, 49, 52, 54, 55, 53, },
				HexVertData<I>{56, 58, 59, 57, 60, 62, 63, 61, },
				HexVertData<I>{64, 66, 67, 65, 68, 70, 71, 69, },
				HexVertData<I>{70, 72, 73, 71, 74, 76, 77, 75, },
				HexVertData<I>{78, 80, 81, 79, 82, 84, 85, 83, },
				HexVertData<I>{86, 88, 89, 87, 90, 92, 93, 91, },
				HexVertData<I>{94, 96, 97, 95, 98, 100, 101, 99, },
				HexVertData<I>{102, 103, 104, 66, 105, 106, 72, 70, },
				HexVertData<I>{107, 109, 110, 108, 111, 113, 114, 112, },
				HexVertData<I>{50, 8, 116, 115, 70, 114, 117, 82, },
			}
		};

		const Scheme inset{
			3,
			std::vector<IVec>{
				IVec{2, 1, 0},
				IVec{3, 0, 0},
				IVec{2, 2, 0},
				IVec{3, 3, 0},
				IVec{2, 1, 1},
				IVec{3, 0, 3},
				IVec{2, 2, 1},
				IVec{3, 3, 3},
				IVec{0, 0, 0},
				IVec{1, 1, 0},
				IVec{0, 3, 0},
				IVec{1, 2, 0},
				IVec{0, 0, 3},
				IVec{1, 1, 1},
				IVec{0, 3, 3},
				IVec{1, 2, 1},
			},
			std::vector<HexVertData<I>>{
				HexVertData<I>{0, 2, 3, 1, 4, 6, 7, 5, },
				HexVertData<I>{8, 10, 11, 9, 12, 14, 15, 13, },
				HexVertData<I>{11, 10, 3, 2, 15, 14, 7, 6, },
				HexVertData<I>{8, 9, 0, 1, 12, 13, 4, 5, },
				HexVertData<I>{13, 15, 6, 4, 12, 14, 7, 5, },
				HexVertData<I>{9, 11, 2, 0, 13, 15, 6, 4, },
			}
		};

		const Scheme subdivide3x3{
			3,
			std::vector<IVec>{
				IVec{0, 0, 0},
				IVec{1, 0, 0},
				IVec{0, 1, 0},
				IVec{1, 1, 0},
				IVec{0, 0, 1},
				IVec{1, 0, 1},
				IVec{0, 1, 1},
				IVec{1, 1, 1},
				IVec{0, 0, 2},
				IVec{1, 0, 2},
				IVec{0, 1, 2},
				IVec{1, 1, 2},
				IVec{0, 0, 3},
				IVec{1, 0, 3},
				IVec{0, 1, 3},
				IVec{1, 1, 3},
				IVec{0, 2, 0},
				IVec{1, 2, 0},
				IVec{0, 2, 1},
				IVec{1, 2, 1},
				IVec{0, 2, 2},
				IVec{1, 2, 2},
				IVec{0, 2, 3},
				IVec{1, 2, 3},
				IVec{0, 3, 0},
				IVec{1, 3, 0},
				IVec{0, 3, 1},
				IVec{1, 3, 1},
				IVec{0, 3, 2},
				IVec{1, 3, 2},
				IVec{0, 3, 3},
				IVec{1, 3, 3},
				IVec{2, 0, 0},
				IVec{2, 1, 0},
				IVec{2, 0, 1},
				IVec{2, 1, 1},
				IVec{2, 0, 2},
				IVec{2, 1, 2},
				IVec{2, 0, 3},
				IVec{2, 1, 3},
				IVec{2, 2, 0},
				IVec{2, 2, 1},
				IVec{2, 2, 2},
				IVec{2, 2, 3},
				IVec{2, 3, 0},
				IVec{2, 3, 1},
				IVec{2, 3, 2},
				IVec{2, 3, 3},
				IVec{3, 0, 0},
				IVec{3, 1, 0},
				IVec{3, 0, 1},
				IVec{3, 1, 1},
				IVec{3, 0, 2},
				IVec{3, 1, 2},
				IVec{3, 0, 3},
				IVec{3, 1, 3},
				IVec{3, 2, 0},
				IVec{3, 2, 1},
				IVec{3, 2, 2},
				IVec{3, 2, 3},
				IVec{3, 3, 0},
				IVec{3, 3, 1},
				IVec{3, 3, 2},
				IVec{3, 3, 3},
			},
			std::vector<HexVertData<I>>{
				HexVertData<I>{0, 2, 3, 1, 4, 6, 7, 5, },
				HexVertData<I>{4, 6, 7, 5, 8, 10, 11, 9, },
				HexVertData<I>{8, 10, 11, 9, 12, 14, 15, 13, },
				HexVertData<I>{2, 16, 17, 3, 6, 18, 19, 7, },
				HexVertData<I>{6, 18, 19, 7, 10, 20, 21, 11, },
				HexVertData<I>{10, 20, 21, 11, 14, 22, 23, 15, },
				HexVertData<I>{16, 24, 25, 17, 18, 26, 27, 19, },
				HexVertData<I>{18, 26, 27, 19, 20, 28, 29, 21, },
				HexVertData<I>{20, 28, 29, 21, 22, 30, 31, 23, },
				HexVertData<I>{1, 3, 33, 32, 5, 7, 35, 34, },
				HexVertData<I>{5, 7, 35, 34, 9, 11, 37, 36, },
				HexVertData<I>{9, 11, 37, 36, 13, 15, 39, 38, },
				HexVertData<I>{3, 17, 40, 33, 7, 19, 41, 35, },
				HexVertData<I>{7, 19, 41, 35, 11, 21, 42, 37, },
				HexVertData<I>{11, 21, 42, 37, 15, 23, 43, 39, },
				HexVertData<I>{17, 25, 44, 40, 19, 27, 45, 41, },
				HexVertData<I>{19, 27, 45, 41, 21, 29, 46, 42, },
				HexVertData<I>{21, 29, 46, 42, 23, 31, 47, 43, },
				HexVertData<I>{32, 33, 49, 48, 34, 35, 51, 50, },
				HexVertData<I>{34, 35, 51, 50, 36, 37, 53, 52, },
				HexVertData<I>{36, 37, 53, 52, 38, 39, 55, 54, },
				HexVertData<I>{33, 40, 56, 49, 35, 41, 57, 51, },
				HexVertData<I>{35, 41, 57, 51, 37, 42, 58, 53, },
				HexVertData<I>{37, 42, 58, 53, 39, 43, 59, 55, },
				HexVertData<I>{40, 44, 60, 56, 41, 45, 61, 57, },
				HexVertData<I>{41, 45, 61, 57, 42, 46, 62, 58, },
				HexVertData<I>{42, 46, 62, 58, 43, 47, 63, 59, },
			}
		};

		const Scheme adapterEdgeSubdivide3x3{
			3,
			std::vector<IVec>{
				IVec{1, 2, 0},
				IVec{2, 2, 0},
				IVec{0, 3, 0},
				IVec{3, 3, 0},
				IVec{1, 2, 2},
				IVec{2, 2, 2},
				IVec{0, 3, 3},
				IVec{3, 3, 3},
				IVec{0, 0, 0},
				IVec{1, 0, 0},
				IVec{0, 0, 3},
				IVec{1, 0, 2},
				IVec{2, 0, 0},
				IVec{3, 0, 0},
				IVec{2, 0, 2},
				IVec{3, 0, 3},
			},
			std::vector<HexVertData<I>>{
				HexVertData<I>{0, 2, 3, 1, 4, 6, 7, 5, },
				HexVertData<I>{8, 2, 0, 9, 10, 6, 4, 11, },
				HexVertData<I>{12, 1, 3, 13, 14, 5, 7, 15, },
				HexVertData<I>{11, 4, 5, 14, 10, 6, 7, 15, },
				HexVertData<I>{9, 0, 1, 12, 11, 4, 5, 14, },
			}
		};

		const Scheme adapterFaceSubdivide3x3{
			3,
			std::vector<IVec>{
				IVec{0, 0, 0},
				IVec{1, 0, 0},
				IVec{0, 1, 0},
				IVec{1, 1, 0},
				IVec{0, 0, 3},
				IVec{1, 0, 2},
				IVec{0, 1, 2},
				IVec{1, 1, 1},
				IVec{0, 2, 0},
				IVec{1, 2, 0},
				IVec{0, 3, 0},
				IVec{1, 3, 0},
				IVec{0, 2, 2},
				IVec{1, 2, 1},
				IVec{0, 3, 3},
				IVec{1, 3, 2},
				IVec{2, 0, 0},
				IVec{3, 0, 0},
				IVec{2, 1, 0},
				IVec{3, 1, 0},
				IVec{2, 0, 2},
				IVec{3, 0, 3},
				IVec{2, 1, 1},
				IVec{3, 1, 2},
				IVec{2, 2, 0},
				IVec{3, 2, 0},
				IVec{2, 3, 0},
				IVec{3, 3, 0},
				IVec{2, 2, 1},
				IVec{3, 2, 2},
				IVec{2, 3, 2},
				IVec{3, 3, 3},
			},
			std::vector<HexVertData<I>>{
				HexVertData<I>{0, 2, 3, 1, 4, 6, 7, 5, },
				HexVertData<I>{8, 10, 11, 9, 12, 14, 15, 13, },
				HexVertData<I>{2, 8, 9, 3, 6, 12, 13, 7, },
				HexVertData<I>{6, 12, 13, 7, 4, 14, 15, 5, },
				HexVertData<I>{16, 18, 19, 17, 20, 22, 23, 21, },
				HexVertData<I>{24, 26, 27, 25, 28, 30, 31, 29, },
				HexVertData<I>{18, 24, 25, 19, 22, 28, 29, 23, },
				HexVertData<I>{22, 28, 29, 23, 20, 30, 31, 21, },
				HexVertData<I>{7, 13, 28, 22, 5, 15, 30, 20, },
				HexVertData<I>{1, 3, 18, 16, 5, 7, 22, 20, },
				HexVertData<I>{3, 9, 24, 18, 7, 13, 28, 22, },
				HexVertData<I>{9, 11, 26, 24, 13, 15, 30, 28, },
				HexVertData<I>{5, 15, 30, 20, 4, 14, 31, 21, },
			}
		};

		const Scheme adapter2FacesSubdivide3x3{
			3,
			std::vector<IVec>{
				IVec{0, 0, 0},
				IVec{1, 0, 0},
				IVec{0, 1, 0},
				IVec{1, 1, 0},
				IVec{0, 0, 1},
				IVec{1, 0, 1},
				IVec{0, 1, 1},
				IVec{1, 1, 1},
				IVec{2, 0, 0},
				IVec{2, 1, 0},
				IVec{2, 0, 1},
				IVec{2, 1, 1},
				IVec{3, 0, 0},
				IVec{3, 1, 0},
				IVec{3, 0, 1},
				IVec{3, 1, 1},
				IVec{0, 0, 2},
				IVec{1, 0, 2},
				IVec{0, 2, 2},
				IVec{1, 2, 2},
				IVec{2, 0, 2},
				IVec{2, 2, 2},
				IVec{3, 0, 2},
				IVec{3, 2, 2},
				IVec{0, 2, 0},
				IVec{1, 2, 0},
				IVec{2, 2, 0},
				IVec{3, 2, 0},
				IVec{0, 3, 0},
				IVec{1, 3, 0},
				IVec{0, 3, 3},
				IVec{1, 3, 2},
				IVec{2, 3, 0},
				IVec{2, 3, 2},
				IVec{3, 3, 0},
				IVec{3, 3, 3},
				IVec{0, 0, 3},
				IVec{1, 0, 3},
				IVec{1, 2, 3},
				IVec{2, 0, 3},
				IVec{2, 2, 3},
				IVec{3, 0, 3},
			},
			std::vector<HexVertData<I>>{
				HexVertData<I>{0, 2, 3, 1, 4, 6, 7, 5, },
				HexVertData<I>{1, 3, 9, 8, 5, 7, 11, 10, },
				HexVertData<I>{8, 9, 13, 12, 10, 11, 15, 14, },
				HexVertData<I>{4, 6, 7, 5, 16, 18, 19, 17, },
				HexVertData<I>{5, 7, 11, 10, 17, 19, 21, 20, },
				HexVertData<I>{10, 11, 15, 14, 20, 21, 23, 22, },
				HexVertData<I>{2, 24, 25, 3, 6, 18, 19, 7, },
				HexVertData<I>{3, 25, 26, 9, 7, 19, 21, 11, },
				HexVertData<I>{9, 26, 27, 13, 11, 21, 23, 15, },
				HexVertData<I>{24, 28, 29, 25, 18, 30, 31, 19, },
				HexVertData<I>{25, 29, 32, 26, 19, 31, 33, 21, },
				HexVertData<I>{26, 32, 34, 27, 21, 33, 35, 23, },
				HexVertData<I>{16, 18, 19, 17, 36, 30, 38, 37, },
				HexVertData<I>{17, 19, 21, 20, 37, 38, 40, 39, },
				HexVertData<I>{20, 21, 23, 22, 39, 40, 35, 41, },
				HexVertData<I>{19, 31, 33, 21, 38, 30, 35, 40, },
			}
		};

	}

	const std::unordered_map<EScheme, const Scheme&> schemes{
		{EScheme::Subdivide3x3, Schemes::subdivide3x3},
		{EScheme::AdapterFaceSubdivide3x3, Schemes::adapterFaceSubdivide3x3},
		{EScheme::Adapter2FacesSubdivide3x3, Schemes::adapter2FacesSubdivide3x3},
		{EScheme::AdapterEdgeSubdivide3x3, Schemes::adapterEdgeSubdivide3x3},
		{EScheme::Inset, Schemes::inset},
		{EScheme::Test, Schemes::test}
	};

}