#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Operation.hpp>
#include <array>

namespace HMP::Dag
{

	class Extrude final: public Operation
	{

	public:

		enum class ESource
		{
			Face, Edge, Vertex
		};

	private:

		std::array<Id, 3> m_faceOffsets;
		ESource m_source;

	public:

		Extrude();

		Id forwardFaceOffset() const;
		Id& forwardFaceOffset();

		Id upFaceOffset() const;
		Id& upFaceOffset();

		Id rightFaceOffset() const;
		Id& rightFaceOffset();

		const std::array<Id, 3>& faceOffsets() const;
		std::array<Id, 3>& faceOffsets();

		ESource source() const;
		ESource& source();

	};

}