#include <HMP/Utils/Transform.hpp>
#include <unordered_map>
#include <array>

namespace HMP::Utils::Transform
{

	unsigned int rotateVid(EAxis _axis, unsigned int _id, int _times)
	{

		static const std::unordered_map<EAxis, std::array<unsigned int, 8>> rotationAxisMap{
			{EAxis::X, {3,2,6,7,0,1,5,4}},
			{EAxis::Y, {3,0,1,2,7,4,5,6}},
			{EAxis::Z, {4,0,3,7,5,1,2,6}},
		};

		const std::array<unsigned int, 8>& rotationMap{ rotationAxisMap.at(_axis) };

		while (_times-- > 0)
		{
			_id = rotationMap[_id];
		}

		return _id;
	}

}