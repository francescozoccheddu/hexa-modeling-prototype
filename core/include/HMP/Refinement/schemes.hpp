#pragma once

#include <HMP/types.hpp>
#include <HMP/Refinement/Scheme.hpp>
#include <unordered_map>

namespace HMP::Refinement
{

	enum class EScheme
	{
		Subdivide3x3, InterfaceFace, InterfaceEdge, Inset
	};

	extern const std::unordered_map<EScheme, const Scheme*> schemes;

	namespace Schemes
	{

		extern const Scheme subdivide3x3;
		extern const Scheme interfaceFace;
		extern const Scheme interfaceEdge;
		extern const Scheme inset;

	}

}