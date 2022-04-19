#pragma once

namespace HMP::Utils
{

	enum class EAxis
	{
		X, Y, Z
	};

	unsigned int rotateVertices(EAxis _axis, unsigned int _id, int _times);

}