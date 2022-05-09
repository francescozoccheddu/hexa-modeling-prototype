#include <HMP/Meshing/refinementSchemes.hpp>

namespace HMP::Meshing
{

	namespace RefinementSchemes
	{

		const Refinement inset{ {
			{{
				{{5, -0}, {7, 1}, {4, -0}, {3, -0}, },
				{{6, 1}, {2, -0}, {3, -0}, {5, -0}, },
				{{5, 1}, {7, -0}, {4, -0}, {3, -0}, },
				{{5, -0}, {7, -0}, {4, 1}, {3, -0}, },
				{{5, 0.333333}, {7, -0}, {4, -0}, {3, 0.666667}, },
				{{5, 0.333333}, {2, 0.333333}, {3, 0.333333}, {1, -0}, },
				{{5, 0.333333}, {0, -0}, {1, 0.333333}, {3, 0.333333}, },
				{{5, 0.333333}, {4, -0}, {0, 0.333333}, {3, 0.333333}, },
			}},
			{{
				{{5, 0.333333}, {7, -0}, {4, -0}, {3, 0.666667}, },
				{{5, 0.333333}, {2, 0.333333}, {3, 0.333333}, {1, -0}, },
				{{5, 0.333333}, {0, -0}, {1, 0.333333}, {3, 0.333333}, },
				{{5, 0.333333}, {4, -0}, {0, 0.333333}, {3, 0.333333}, },
				{{5, -0}, {0, -0}, {1, 0.333333}, {3, 0.666667}, },
				{{5, -0}, {2, 0.333333}, {3, 0.333333}, {1, 0.333333}, },
				{{5, -0}, {0, -0}, {1, 0.666667}, {3, 0.333333}, },
				{{5, -0}, {0, 0.333333}, {1, 0.333333}, {3, 0.333333}, },
			}},
			{{
				{{5, 0.333333}, {2, 0.333333}, {3, 0.333333}, {1, -0}, },
				{{6, 1}, {2, -0}, {3, -0}, {5, -0}, },
				{{5, 1}, {7, -0}, {4, -0}, {3, -0}, },
				{{5, 0.333333}, {0, -0}, {1, 0.333333}, {3, 0.333333}, },
				{{5, -0}, {2, 0.333333}, {3, 0.333333}, {1, 0.333333}, },
				{{5, -0}, {2, 1}, {3, -0}, {1, -0}, },
				{{5, -0}, {0, -0}, {1, 1}, {3, -0}, },
				{{5, -0}, {0, -0}, {1, 0.666667}, {3, 0.333333}, },
			}},
			{{
				{{5, -0}, {7, 1}, {4, -0}, {3, -0}, },
				{{6, 1}, {2, -0}, {3, -0}, {5, -0}, },
				{{5, 0.333333}, {2, 0.333333}, {3, 0.333333}, {1, -0}, },
				{{5, 0.333333}, {7, -0}, {4, -0}, {3, 0.666667}, },
				{{5, -0}, {7, -0}, {4, -0}, {3, 1}, },
				{{5, -0}, {2, 1}, {3, -0}, {1, -0}, },
				{{5, -0}, {2, 0.333333}, {3, 0.333333}, {1, 0.333333}, },
				{{5, -0}, {0, -0}, {1, 0.333333}, {3, 0.666667}, },
			}},
			{{
				{{5, -0}, {7, 1}, {4, -0}, {3, -0}, },
				{{5, 0.333333}, {7, -0}, {4, -0}, {3, 0.666667}, },
				{{5, 0.333333}, {4, -0}, {0, 0.333333}, {3, 0.333333}, },
				{{5, -0}, {7, -0}, {4, 1}, {3, -0}, },
				{{5, -0}, {7, -0}, {4, -0}, {3, 1}, },
				{{5, -0}, {0, -0}, {1, 0.333333}, {3, 0.666667}, },
				{{5, -0}, {0, 0.333333}, {1, 0.333333}, {3, 0.333333}, },
				{{5, -0}, {4, -0}, {0, 1}, {3, -0}, },
			}},
			{{
				{{5, 0.333333}, {4, -0}, {0, 0.333333}, {3, 0.333333}, },
				{{5, 0.333333}, {0, -0}, {1, 0.333333}, {3, 0.333333}, },
				{{5, 1}, {7, -0}, {4, -0}, {3, -0}, },
				{{5, -0}, {7, -0}, {4, 1}, {3, -0}, },
				{{5, -0}, {0, 0.333333}, {1, 0.333333}, {3, 0.333333}, },
				{{5, -0}, {0, -0}, {1, 0.666667}, {3, 0.333333}, },
				{{5, -0}, {0, -0}, {1, 1}, {3, -0}, },
				{{5, -0}, {4, -0}, {0, 1}, {3, -0}, },
			}},
		} };

		const Refinement subdivide3x3{ {
			{{
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{2, 0.333333}, {7, 0.666667}, {3, 2.77556e-17}, {1, -0}, },
				{{1, -0}, {3, 0.333333}, {0, -0}, {7, 0.666667}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {4, -0}, {5, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {6, 0.333333}, {7, 0.666667}, {5, -0}, },
				{{1, -0}, {3, -0}, {0, -0}, {7, 1}, },
				{{1, -0}, {0, -0}, {4, 0.333333}, {7, 0.666667}, },
			}},
			{{
				{{1, 1}, {3, -0}, {0, -0}, {7, -0}, },
				{{2, 0.333333}, {6, -0}, {7, -0}, {1, 0.666667}, },
				{{1, 0.666667}, {3, 0.333333}, {0, -0}, {7, -0}, },
				{{1, 0.666667}, {3, -0}, {0, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {4, -0}, {5, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {6, 0.333333}, {7, -0}, {5, -0}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{1, 0.666667}, {4, 0.333333}, {5, 2.77556e-17}, {7, -0}, },
			}},
			{{
				{{2, 0.333333}, {6, -0}, {7, -0}, {1, 0.666667}, },
				{{2, 0.666667}, {6, -0}, {7, -0}, {1, 0.333333}, },
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{1, 0.666667}, {3, 0.333333}, {0, -0}, {7, -0}, },
				{{1, 0.666667}, {6, 0.333333}, {7, -0}, {5, -0}, },
				{{2, 0.333333}, {6, 0.333333}, {7, -0}, {1, 0.333333}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
			}},
			{{
				{{2, 0.666667}, {6, -0}, {7, -0}, {1, 0.333333}, },
				{{2, 1}, {6, -0}, {7, -0}, {1, -0}, },
				{{2, 0.666667}, {7, -0}, {3, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{2, 0.333333}, {6, 0.333333}, {7, -0}, {1, 0.333333}, },
				{{2, 0.666667}, {6, 0.333333}, {7, -0}, {1, -0}, },
				{{2, 0.666667}, {6, 2.77556e-17}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
			}},
			{{
				{{1, 0.666667}, {3, -0}, {0, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {3, 0.333333}, {0, -0}, {7, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, 0.666667}, {7, -0}, },
				{{1, 0.666667}, {4, 0.333333}, {5, 2.77556e-17}, {7, -0}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {0, 0.333333}, {4, 0.333333}, {7, -0}, },
			}},
			{{
				{{1, 0.666667}, {3, 0.333333}, {0, -0}, {7, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{2, 0.666667}, {7, -0}, {3, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.666667}, {1, -0}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{2, 0.666667}, {6, 2.77556e-17}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {7, 0.333333}, {3, 0.333333}, {1, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.333333}, {3, -0}, {0, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, 0.333333}, {7, -0}, },
				{{1, -0}, {3, 0.333333}, {0, 0.666667}, {7, -0}, },
				{{1, -0}, {3, -0}, {0, 1}, {7, -0}, },
				{{1, 0.333333}, {0, 0.333333}, {4, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {3, -0}, {0, 0.666667}, {7, 0.333333}, },
				{{1, -0}, {0, 0.666667}, {4, 0.333333}, {7, -0}, },
			}},
			{{
				{{1, 0.333333}, {3, 0.333333}, {0, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{1, -0}, {3, 0.666667}, {0, 0.333333}, {7, -0}, },
				{{1, -0}, {3, 0.333333}, {0, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, -0}, {3, 0.333333}, {0, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {3, -0}, {0, 0.666667}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.666667}, {1, -0}, },
				{{1, -0}, {3, 1}, {0, -0}, {7, -0}, },
				{{1, -0}, {3, 0.666667}, {0, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{2, 0.333333}, {7, 0.333333}, {3, 0.333333}, {1, -0}, },
				{{1, -0}, {3, 0.666667}, {0, -0}, {7, 0.333333}, },
				{{1, -0}, {3, 0.333333}, {0, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.666667}, {4, -0}, {5, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {6, 0.333333}, {7, -0}, {5, -0}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{1, 0.666667}, {4, 0.333333}, {5, 2.77556e-17}, {7, -0}, },
				{{1, 0.333333}, {4, -0}, {5, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, 0.333333}, {4, -0}, {5, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
			}},
			{{
				{{1, 0.666667}, {6, 0.333333}, {7, -0}, {5, -0}, },
				{{2, 0.333333}, {6, 0.333333}, {7, -0}, {1, 0.333333}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, 0.333333}, {4, -0}, {5, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{2, 0.333333}, {6, 0.333333}, {7, -0}, {1, 0.333333}, },
				{{2, 0.666667}, {6, 0.333333}, {7, -0}, {1, -0}, },
				{{2, 0.666667}, {6, 2.77556e-17}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{2, 0.333333}, {6, 0.666667}, {7, -0}, {1, -0}, },
				{{2, 0.333333}, {6, 0.333333}, {7, 0.333333}, {1, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
			}},
			{{
				{{1, 0.666667}, {4, 0.333333}, {5, 2.77556e-17}, {7, -0}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {0, 0.333333}, {4, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {4, -0}, {5, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {0, -0}, {4, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {0, 2.77556e-17}, {4, 0.666667}, {7, -0}, },
			}},
			{{
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {4, -0}, {5, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{1, 0.333333}, {0, -0}, {4, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{2, 0.666667}, {6, 2.77556e-17}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {7, 0.333333}, {3, 0.333333}, {1, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{2, 0.333333}, {6, 0.333333}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {7, 0.666667}, {3, 2.77556e-17}, {1, -0}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
			}},
			{{
				{{1, 0.333333}, {0, 0.333333}, {4, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {3, -0}, {0, 0.666667}, {7, 0.333333}, },
				{{1, -0}, {0, 0.666667}, {4, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {0, 2.77556e-17}, {4, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {0, -0}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {0, 0.333333}, {4, 0.666667}, {7, -0}, },
			}},
			{{
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, -0}, {3, 0.333333}, {0, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {3, -0}, {0, 0.666667}, {7, 0.333333}, },
				{{1, 0.333333}, {0, -0}, {4, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{2, 0.333333}, {7, 0.333333}, {3, 0.333333}, {1, -0}, },
				{{1, -0}, {3, 0.666667}, {0, -0}, {7, 0.333333}, },
				{{1, -0}, {3, 0.333333}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{2, 0.333333}, {7, 0.666667}, {3, 2.77556e-17}, {1, -0}, },
				{{1, -0}, {3, 0.333333}, {0, -0}, {7, 0.666667}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
			}},
			{{
				{{1, 0.333333}, {4, -0}, {5, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, 0.333333}, {4, -0}, {5, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
				{{1, -0}, {4, -0}, {5, 1}, {7, -0}, },
				{{1, -0}, {6, 0.333333}, {7, -0}, {5, 0.666667}, },
				{{1, -0}, {4, -0}, {5, 0.666667}, {7, 0.333333}, },
				{{1, -0}, {4, 0.333333}, {5, 0.666667}, {7, -0}, },
			}},
			{{
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, 0.333333}, {4, -0}, {5, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {6, 0.333333}, {7, -0}, {5, 0.666667}, },
				{{1, -0}, {6, 0.666667}, {7, -0}, {5, 0.333333}, },
				{{1, -0}, {6, 0.333333}, {7, 0.333333}, {5, 0.333333}, },
				{{1, -0}, {4, -0}, {5, 0.666667}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{2, 0.333333}, {6, 0.666667}, {7, -0}, {1, -0}, },
				{{2, 0.333333}, {6, 0.333333}, {7, 0.333333}, {1, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, -0}, {6, 0.666667}, {7, -0}, {5, 0.333333}, },
				{{1, -0}, {6, 1}, {7, -0}, {5, -0}, },
				{{1, -0}, {6, 0.666667}, {7, 0.333333}, {5, -0}, },
				{{1, -0}, {6, 0.333333}, {7, 0.333333}, {5, 0.333333}, },
			}},
			{{
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {4, -0}, {5, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {0, -0}, {4, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {0, 2.77556e-17}, {4, 0.666667}, {7, -0}, },
				{{1, -0}, {4, 0.333333}, {5, 0.666667}, {7, -0}, },
				{{1, -0}, {4, -0}, {5, 0.666667}, {7, 0.333333}, },
				{{1, -0}, {4, 0.333333}, {5, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {4, 0.666667}, {5, 0.333333}, {7, -0}, },
			}},
			{{
				{{1, 0.333333}, {4, -0}, {5, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{1, 0.333333}, {0, -0}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {4, -0}, {5, 0.666667}, {7, 0.333333}, },
				{{1, -0}, {6, 0.333333}, {7, 0.333333}, {5, 0.333333}, },
				{{1, -0}, {4, -0}, {5, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {4, 0.333333}, {5, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{2, 0.333333}, {6, 0.333333}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {7, 0.666667}, {3, 2.77556e-17}, {1, -0}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{1, -0}, {6, 0.333333}, {7, 0.333333}, {5, 0.333333}, },
				{{1, -0}, {6, 0.666667}, {7, 0.333333}, {5, -0}, },
				{{1, -0}, {6, 0.333333}, {7, 0.666667}, {5, -0}, },
				{{1, -0}, {4, -0}, {5, 0.333333}, {7, 0.666667}, },
			}},
			{{
				{{1, 0.333333}, {0, 2.77556e-17}, {4, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {0, -0}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {0, 0.333333}, {4, 0.666667}, {7, -0}, },
				{{1, -0}, {4, 0.666667}, {5, 0.333333}, {7, -0}, },
				{{1, -0}, {4, 0.333333}, {5, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {0, -0}, {4, 0.666667}, {7, 0.333333}, },
				{{1, -0}, {0, -0}, {4, 1}, {7, -0}, },
			}},
			{{
				{{1, 0.333333}, {0, -0}, {4, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {4, 0.333333}, {5, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {4, -0}, {5, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {0, -0}, {4, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {0, -0}, {4, 0.666667}, {7, 0.333333}, },
			}},
		} };

		const Refinement adapterEdgeSubdivide3x3{ {
			{{
				{{1, 0.333333}, {3, -0}, {0, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{1, -0}, {3, 1}, {0, -0}, {7, -0}, },
				{{1, -0}, {3, -0}, {0, 1}, {7, -0}, },
				{{1, 0.333333}, {0, -0}, {4, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{1, -0}, {3, -0}, {0, -0}, {7, 1}, },
				{{1, -0}, {0, -0}, {4, 1}, {7, -0}, },
			}},
			{{
				{{1, 0.666667}, {3, -0}, {0, 0.333333}, {7, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{1, 0.333333}, {0, -0}, {4, 0.666667}, {7, -0}, },
			}},
			{{
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
				{{1, 0.333333}, {0, -0}, {4, 0.666667}, {7, -0}, },
				{{1, -0}, {4, -0}, {5, 1}, {7, -0}, },
				{{1, -0}, {6, 1}, {7, -0}, {5, -0}, },
				{{1, -0}, {3, -0}, {0, -0}, {7, 1}, },
				{{1, -0}, {0, -0}, {4, 1}, {7, -0}, },
			}},
			{{
				{{1, 1}, {3, -0}, {0, -0}, {7, -0}, },
				{{2, 1}, {6, -0}, {7, -0}, {1, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{1, 0.666667}, {3, -0}, {0, 0.333333}, {7, -0}, },
				{{1, -0}, {4, -0}, {5, 1}, {7, -0}, },
				{{1, -0}, {6, 1}, {7, -0}, {5, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
			}},
			{{
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{2, 1}, {6, -0}, {7, -0}, {1, -0}, },
				{{1, -0}, {3, 1}, {0, -0}, {7, -0}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, 0.333333}, {5, -0}, },
				{{1, -0}, {6, 1}, {7, -0}, {5, -0}, },
				{{1, -0}, {3, -0}, {0, -0}, {7, 1}, },
				{{1, 0.333333}, {3, -0}, {0, -0}, {7, 0.666667}, },
			}},
		} };

		const Refinement adapterFaceSubdivide3x3{ {
			{{
				{{1, 0.333333}, {3, 0.333333}, {0, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{1, -0}, {3, 0.666667}, {0, 0.333333}, {7, -0}, },
				{{1, -0}, {3, 0.333333}, {0, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.666667}, {3, 0.333333}, {0, -0}, {7, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{2, 0.333333}, {6, -0}, {7, -0}, {1, 0.666667}, },
				{{2, 0.666667}, {6, -0}, {7, -0}, {1, 0.333333}, },
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{1, 0.666667}, {3, 0.333333}, {0, -0}, {7, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
			}},
			{{
				{{1, 0.666667}, {3, -0}, {0, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {3, 0.333333}, {0, -0}, {7, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {0, -0}, {4, 0.666667}, {7, -0}, },
			}},
			{{
				{{1, 1}, {3, -0}, {0, -0}, {7, -0}, },
				{{2, 0.333333}, {6, -0}, {7, -0}, {1, 0.666667}, },
				{{1, 0.666667}, {3, 0.333333}, {0, -0}, {7, -0}, },
				{{1, 0.666667}, {3, -0}, {0, 0.333333}, {7, -0}, },
				{{1, -0}, {4, -0}, {5, 1}, {7, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
			}},
			{{
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{2, 0.666667}, {7, -0}, {3, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.666667}, {1, -0}, },
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{2, 0.333333}, {6, 0.333333}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {6, -0}, {7, 0.666667}, {1, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
			}},
			{{
				{{2, 0.666667}, {6, -0}, {7, -0}, {1, 0.333333}, },
				{{2, 1}, {6, -0}, {7, -0}, {1, -0}, },
				{{2, 0.666667}, {7, -0}, {3, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.333333}, {1, 0.333333}, },
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{1, -0}, {6, 1}, {7, -0}, {5, -0}, },
				{{2, 0.333333}, {6, 0.333333}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
			}},
			{{
				{{1, 0.333333}, {3, -0}, {0, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, 0.333333}, {7, -0}, },
				{{1, -0}, {3, 0.333333}, {0, 0.666667}, {7, -0}, },
				{{1, -0}, {3, -0}, {0, 1}, {7, -0}, },
				{{1, 0.333333}, {0, -0}, {4, 0.666667}, {7, -0}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {0, -0}, {4, 1}, {7, -0}, },
			}},
			{{
				{{1, 0.333333}, {3, 0.666667}, {0, -0}, {7, -0}, },
				{{2, 0.333333}, {7, -0}, {3, 0.666667}, {1, -0}, },
				{{1, -0}, {3, 1}, {0, -0}, {7, -0}, },
				{{1, -0}, {3, 0.666667}, {0, 0.333333}, {7, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{2, 0.333333}, {6, -0}, {7, 0.666667}, {1, -0}, },
				{{1, -0}, {3, -0}, {0, -0}, {7, 1}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
			}},
			{{
				{{2, 0.333333}, {6, -0}, {7, 0.333333}, {1, 0.333333}, },
				{{2, 0.333333}, {6, 0.333333}, {7, 0.333333}, {1, -0}, },
				{{2, 0.333333}, {6, -0}, {7, 0.666667}, {1, -0}, },
				{{1, 0.333333}, {3, 0.333333}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{1, -0}, {6, 1}, {7, -0}, {5, -0}, },
				{{1, -0}, {3, -0}, {0, -0}, {7, 1}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
			}},
			{{
				{{1, 0.333333}, {4, 0.333333}, {5, 0.333333}, {7, -0}, },
				{{1, 0.666667}, {3, -0}, {0, -0}, {7, 0.333333}, },
				{{1, 0.333333}, {3, -0}, {0, 0.333333}, {7, 0.333333}, },
				{{1, 0.333333}, {0, -0}, {4, 0.666667}, {7, -0}, },
				{{1, -0}, {4, -0}, {5, 1}, {7, -0}, },
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {0, -0}, {4, 1}, {7, -0}, },
			}},
			{{
				{{1, 0.333333}, {6, 0.333333}, {7, -0}, {5, 0.333333}, },
				{{1, 0.333333}, {6, 0.666667}, {7, -0}, {5, -0}, },
				{{1, -0}, {3, -0}, {0, 0.333333}, {7, 0.666667}, },
				{{1, -0}, {0, 0.333333}, {4, 0.333333}, {7, 0.333333}, },
				{{1, -0}, {4, -0}, {5, 1}, {7, -0}, },
				{{1, -0}, {6, 1}, {7, -0}, {5, -0}, },
				{{1, -0}, {3, -0}, {0, -0}, {7, 1}, },
				{{1, -0}, {0, -0}, {4, 1}, {7, -0}, },
			}},
		} };

	}

	const std::unordered_map<ERefinementScheme, const Refinement&> refinementSchemes{
		{ERefinementScheme::Subdivide3x3, RefinementSchemes::subdivide3x3},
		{ERefinementScheme::AdapterFaceSubdivide3x3, RefinementSchemes::adapterFaceSubdivide3x3},
		{ERefinementScheme::AdapterEdgeSubdivide3x3, RefinementSchemes::adapterEdgeSubdivide3x3},
		{ERefinementScheme::Inset, RefinementSchemes::inset}
	};

}