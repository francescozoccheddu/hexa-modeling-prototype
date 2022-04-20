#pragma once

namespace HMP::Utils::Transform
{

	enum class EAxis
	{
		X, Y, Z
	};

	unsigned int rotateVid(EAxis _axis, unsigned int _id, int _times);

}