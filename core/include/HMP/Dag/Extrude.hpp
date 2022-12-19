#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/Operation.hpp>
#include <cpputils/collections/FixedVector.hpp>

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

		cpputils::collections::FixedVector<Id, 3> m_faceOffsets;
		ESource m_source;

	public:

		Extrude();

		Id forwardFaceOffset() const;

		Id upFaceOffset() const;

		const cpputils::collections::FixedVector<Id, 3>& faceOffsets() const;
		cpputils::collections::FixedVector<Id, 3>& faceOffsets();

		ESource source() const;
		ESource& source();

	};

}