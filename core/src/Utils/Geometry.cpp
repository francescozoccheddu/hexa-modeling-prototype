#include <HMP/Utils/Geometry.hpp>

#include <unordered_map>
#include <array>
#include <algorithm>
#include <utility>

namespace HMP::Utils::Geometry
{

	bool VertexComparer::operator()(const cinolib::vec3d& _a, const cinolib::vec3d& _b) const
	{
		double eps = 1e-6;
		if (_a.x() - _b.x() < 0.0 && abs(_a.x() - _b.x()) > eps)
		{
			return true;
		}
		else if (abs(_a.x() - _b.x()) < eps)
		{
			if (_a.y() - _b.y() < 0.0 && abs(_a.y() - _b.y()) > eps)
			{
				return true;
			}
			else if (abs(_a.y() - _b.y()) < eps)
			{
				if (_a.z() - _b.z() < 0.0 && abs(_a.z() - _b.z()) > eps)
				{
					return true;
				}
			}
		}

		return false;
	}

	template<unsigned int revolution, unsigned int elementsCount>
	unsigned int transform(const std::unordered_map<EAxis, std::array<unsigned int, elementsCount>>& _map, EAxis _axis, unsigned int _element, int _times = 1)
	{
		const std::array<unsigned int, elementsCount>& map{ _map.at(_axis) };

		if (_times < 0)
		{
			_times = revolution - ((-_times) % revolution);
		}
		_times %= revolution;
		while (_times-- > 0)
		{
			_element = map[static_cast<std::size_t>(_element)];
		}

		return _element;
	}

	unsigned int rotateVid(EAxis _axis, unsigned int _vid, int _times)
	{
		static const std::unordered_map<EAxis, std::array<unsigned int, 8>> map{
			{EAxis::X, {3,2,6,7,0,1,5,4}},
			{EAxis::Y, {3,0,1,2,7,4,5,6}},
			{EAxis::Z, {4,0,3,7,5,1,2,6}},
		};
		return transform<4>(map, _axis, _vid, _times);
	}

	unsigned int rotateFid(EAxis _axis, unsigned int _fid, int _times)
	{
		static const std::unordered_map<EAxis, std::array<unsigned int, 6>> map{
			{EAxis::X, {4,1,5,3,2,0}},
			{EAxis::Y, {0,5,2,4,1,3}},
			{EAxis::Z, {3,0,1,2,4,5}},
		};
		return transform<4>(map, _axis, _fid, _times);
	}

	unsigned int reflectFid(EAxis _axis, unsigned int _fid, int _times)
	{
		static const std::unordered_map<EAxis, std::array<unsigned int, 6>> map{
			{EAxis::X, {0,3,2,1,4,5}},
			{EAxis::Y, {2,1,0,3,4,5}},
			{EAxis::Z, {0,1,2,3,5,4}},
		};
		return transform<2>(map, _axis, _fid, _times);
	}

	void sortVids(std::array<unsigned int, 8>& _vids, const std::array<cinolib::vec3d, 8>& _vertices)
	{
		using Vert = std::pair<cinolib::vec3d, unsigned int>;

		struct OrderX final
		{
			inline bool operator() (const Vert& _a, const Vert& _b)
			{
				return _a.first.x() < _b.first.x();
			}
		};

		struct OrderY final
		{
			inline bool operator() (const Vert& _a, const Vert& _b)
			{
				return _a.first.y() < _b.first.y();
			}
		};

		struct OrderZ final
		{
			inline bool operator() (const Vert& _a, const Vert& _b)
			{
				return _a.first.z() < _b.first.z();
			}
		};

		std::array<Vert, 8> tmp;
		for (unsigned int i = 0; i < 8; i++)
		{
			tmp[i] = { _vertices[_vids[i]], _vids[i] };
		}
		std::sort(tmp.begin(), tmp.end(), OrderY());
		std::sort(tmp.begin(), tmp.begin() + 4, OrderX());
		std::sort(tmp.begin() + 4, tmp.end(), OrderX());
		std::sort(tmp.begin(), tmp.begin() + 2, OrderZ());
		std::sort(tmp.begin() + 2, tmp.begin() + 4, OrderZ());
		std::sort(tmp.begin() + 4, tmp.begin() + 6, OrderZ());
		std::sort(tmp.begin() + 6, tmp.end(), OrderZ());
		std::swap(tmp[0], tmp[3]);
		std::swap(tmp[0], tmp[1]);
		std::swap(tmp[4], tmp[7]);
		std::swap(tmp[4], tmp[5]);

		for (std::size_t i{ 0 }; i < 8; i++)
		{
			_vids[i] = tmp[i].second;
		}
	}



}