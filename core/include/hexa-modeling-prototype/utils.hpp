#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/meshes/meshes.h>

void poly_vert_ordering(const std::vector<cinolib::vec3d>& vertices, std::vector<unsigned int>& poly);
void find_matching_parentheses(std::string& s, std::string symbol, std::unordered_map<unsigned int, unsigned int>& dic);
template<typename T>
void print(const T& container)
{
	for (const auto& el : container)
	{
		std::cout << el << ", ";
	}
	std::cout << std::endl;
}

struct vert_compare
{
	bool operator()(const cinolib::vec3d& a, const cinolib::vec3d& b) const
	{

		double eps = 1e-6;
		if (a.x() - b.x() < 0.0 && abs(a.x() - b.x()) > eps)
		{
			return true;
		}
		else if (abs(a.x() - b.x()) < eps)
		{
			if (a.y() - b.y() < 0.0 && abs(a.y() - b.y()) > eps)
			{
				return true;
			}
			else if (abs(a.y() - b.y()) < eps)
			{
				if (a.z() - b.z() < 0.0 && abs(a.z() - b.z()) > eps)
				{
					return true;
				}
			}
		}

		return false;
	}
};