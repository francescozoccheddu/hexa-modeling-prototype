#pragma once

#include <HMP/Refinement/Scheme.hpp>
#include <unordered_map>

namespace HMP::Refinement
{

	enum class EScheme
	{
		StandardRefinement, FaceScheme, EdgeScheme, FaceRefinement
	};

	extern const std::unordered_map<EScheme, const Scheme*> schemes;

	namespace Schemes
	{

		extern const Scheme standardRefinement;
		extern const Scheme faceScheme;
		extern const Scheme edgeScheme;
		extern const Scheme faceRefinement;

	}

}