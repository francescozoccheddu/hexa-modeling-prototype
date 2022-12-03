#include <HMP/Utils/Geometry.hpp>

#include <unordered_map>
#include <array>
#include <algorithm>
#include <utility>

namespace HMP::Utils::Geometry
{

	bool VertexComparer::operator()(const Vec& _a, const Vec& _b) const
	{
		Real eps = 1e-6;
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

	template<Id revolution, Id elementsCount>
	Id transform(const std::unordered_map<EAxis, std::array<Id, elementsCount>>& _map, EAxis _axis, Id _element, int _times = 1)
	{
		const std::array<Id, elementsCount>& map{ _map.at(_axis) };

		if (_times < 0)
		{
			_times = revolution - ((-_times) % revolution);
		}
		_times %= revolution;
		while (_times-- > 0)
		{
			_element = map[static_cast<I>(_element)];
		}

		return _element;
	}

	Id rotateVid(EAxis _axis, Id _vid, int _times)
	{
		static const std::unordered_map<EAxis, PolyVertIds> map{
			{EAxis::X, {3,2,6,7,0,1,5,4}},
			{EAxis::Y, {3,0,1,2,7,4,5,6}},
			{EAxis::Z, {4,0,3,7,5,1,2,6}},
		};
		return transform<4, 8>(map, _axis, _vid, _times);
	}

	Id rotateFid(EAxis _axis, Id _fid, int _times)
	{
		static const std::unordered_map<EAxis, std::array<Id, 6>> map{
			{EAxis::X, {4,1,5,3,2,0}},
			{EAxis::Y, {0,5,2,4,1,3}},
			{EAxis::Z, {3,0,1,2,4,5}},
		};
		return transform<4, 6>(map, _axis, _fid, _times);
	}

	Id reflectFid(EAxis _axis, Id _fid, int _times)
	{
		static const std::unordered_map<EAxis, std::array<Id, 6>> map{
			{EAxis::X, {0,3,2,1,4,5}},
			{EAxis::Y, {2,1,0,3,4,5}},
			{EAxis::Z, {0,1,2,3,5,4}},
		};
		return transform<2, 6>(map, _axis, _fid, _times);
	}

	void sortVertices(PolyVerts& _verts)
	{

		struct OrderX final
		{
			inline bool operator() (const Vec& _a, const Vec& _b)
			{
				return _a.x() < _b.x();
			}
		};

		struct OrderY final
		{
			inline bool operator() (const Vec& _a, const Vec& _b)
			{
				return _a.y() < _b.y();
			}
		};

		struct OrderZ final
		{
			inline bool operator() (const Vec& _a, const Vec& _b)
			{
				return _a.z() < _b.z();
			}
		};

		std::sort(_verts.begin(), _verts.end(), OrderY());
		std::sort(_verts.begin(), _verts.begin() + 4, OrderX());
		std::sort(_verts.begin() + 4, _verts.end(), OrderX());
		std::sort(_verts.begin(), _verts.begin() + 2, OrderZ());
		std::sort(_verts.begin() + 2, _verts.begin() + 4, OrderZ());
		std::sort(_verts.begin() + 4, _verts.begin() + 6, OrderZ());
		std::sort(_verts.begin() + 6, _verts.end(), OrderZ());
		std::swap(_verts[0], _verts[3]);
		std::swap(_verts[0], _verts[1]);
		std::swap(_verts[4], _verts[7]);
		std::swap(_verts[4], _verts[5]);
	}



}