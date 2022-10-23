#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Refinement.hpp>
#include <unordered_map>

namespace HMP::Meshing
{

	enum class ERefinementScheme
	{
		Subdivide3x3, AdapterFaceSubdivide3x3, Adapter2FacesSubdivide3x3, AdapterEdgeSubdivide3x3, Inset
	};

	extern const std::unordered_map<ERefinementScheme, const Refinement&> refinementSchemes;

	namespace RefinementSchemes
	{

		extern const Refinement subdivide3x3;
		extern const Refinement adapterFaceSubdivide3x3;
		extern const Refinement adapter2FacesSubdivide3x3;
		extern const Refinement adapterEdgeSubdivide3x3;
		extern const Refinement inset;

	}

}