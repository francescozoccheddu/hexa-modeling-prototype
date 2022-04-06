#include <hexa-modeling-prototype/utils.hpp>

#include <stack>
#include <algorithm>

namespace HMP
{

	struct Vert
	{
		cinolib::vec3d coords;
		unsigned int idx;
	};
	struct order_x
	{
		inline bool operator() (const Vert& a, const Vert& b)
		{
			return (a.coords.x() < b.coords.x());
		}
	};
	struct order_y
	{
		inline bool operator() (const Vert& a, const Vert& b)
		{
			return (a.coords.y() < b.coords.y());
		}
	};
	struct order_z
	{
		inline bool operator() (const Vert& a, const Vert& b)
		{
			return (a.coords.z() < b.coords.z());
		}
	};
	void poly_vert_ordering(const std::vector<cinolib::vec3d>& vertices, std::vector<unsigned int>& poly)
	{

		std::vector<Vert> tmp(8);
		for (unsigned int i = 0; i < 8; i++)
		{
			tmp[i] = { vertices[poly[i]], poly[i] };
		}
		std::sort(tmp.begin(), tmp.end(), order_y());
		std::sort(tmp.begin(), tmp.begin() + 4, order_x());
		std::sort(tmp.begin() + 4, tmp.end(), order_x());
		std::sort(tmp.begin(), tmp.begin() + 2, order_z());
		std::sort(tmp.begin() + 2, tmp.begin() + 4, order_z());
		std::sort(tmp.begin() + 4, tmp.begin() + 6, order_z());
		std::sort(tmp.begin() + 6, tmp.end(), order_z());
		std::swap(tmp[0], tmp[3]);
		std::swap(tmp[0], tmp[1]);
		std::swap(tmp[4], tmp[7]);
		std::swap(tmp[4], tmp[5]);

		for (unsigned int i = 0; i < 8; i++)
			poly[i] = tmp[i].idx;
		/*


		cinolib::vec3d centroid(0,0,0);

		for(unsigned int vid : poly){
			centroid += vertices[vid];
		}

		centroid /= poly.size();
		std::vector<unsigned int> tmp_poly(8);

		double eps = 1.0001;

		for(unsigned int vid : poly){
		   const auto &vert = vertices[vid];

				if(vert.x() < centroid.x() && vert.y()<centroid.y() && vert.z()<centroid.z()) tmp_poly[3] = vid;
		   else if(vert.x() < centroid.x() && vert.y()<centroid.y() && vert.z()>centroid.z()) tmp_poly[0] = vid;
		   else if(vert.x() > centroid.x() && vert.y()<centroid.y() && vert.z()>centroid.z()) tmp_poly[1] = vid;
		   else if(vert.x() > centroid.x() && vert.y()<centroid.y() && vert.z()<centroid.z()) tmp_poly[2] = vid;
		   else if(vert.x() < centroid.x() && vert.y()>centroid.y() && vert.z()<centroid.z()) tmp_poly[7] = vid;
		   else if(vert.x() < centroid.x() && vert.y()>centroid.y() && vert.z()>centroid.z()) tmp_poly[4] = vid;
		   else if(vert.x() > centroid.x() && vert.y()>centroid.y() && vert.z()>centroid.z()) tmp_poly[5] = vid;
		   else if(vert.x() > centroid.x() && vert.y()>centroid.y() && vert.z()<centroid.z()) tmp_poly[6] = vid;

		   std::cout<<vert<<" "<<centroid<<std::endl;

		}

		poly = tmp_poly;*/
	}

	void find_matching_parentheses(std::string& s, std::string symbol, std::unordered_map<unsigned int, unsigned int>& dic)
	{
		std::stack<unsigned int> istart;

		for (unsigned int i = 0; i < s.size(); i++)
		{
			char c = s[i];

			if (c == symbol[0])
			{
				istart.push(i);
			}
			if (c == symbol[1])
			{
				if (istart.size() > 0)
				{
					dic[istart.top()] = i;
					istart.pop();
				}
				else
				{
					std::cerr << "Too many closing parentheses" << std::endl;
				}
			}
		}

		if (istart.size() > 0)
		{
			std::cerr << "Too many opening parentheses" << std::endl;
		}

	}

}