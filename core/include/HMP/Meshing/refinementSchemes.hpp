#pragma once

#include <HMP/types.hpp>
#include <HMP/Meshing/Refinement.hpp>
#include <unordered_map>

namespace HMP::Meshing
{

	enum class ERefinementScheme
	{
		Subdivide3x3, InterfaceFace, InterfaceEdge, Inset
	};

	extern const std::unordered_map<ERefinementScheme, const Refinement&> refinementSchemes;

	namespace RefinementSchemes
	{

		extern const Refinement subdivide3x3;
		extern const Refinement interfaceFace;
		extern const Refinement interfaceEdge;
		extern const Refinement inset;

	}

}