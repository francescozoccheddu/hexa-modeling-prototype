#include <HMP/Refinement/Scheme.hpp>

namespace HMP::Refinement
{

	std::vector<std::array<cinolib::vec3d, 8>> Scheme::apply(const std::vector<cinolib::vec3d>& _source) const
	{
		std::vector<std::array<cinolib::vec3d, 8>> polys{};
		polys.reserve(polyCount());
		for (std::size_t i{ 0 }; i < polyCount(); i++)
		{
			std::array<cinolib::vec3d, 8> verts;
			verts.fill(cinolib::vec3d{ 0,0,0 });
			for (std::size_t j{ 0 }; j < 8; j++)
			{
				const auto& vertWeights = weights[i][j];
				const auto& vertOffsets = offsets[i][j];
				for (std::size_t k{ 0 }; k < vertWeights.size(); k++)
				{
					verts[j] += vertWeights[k] * _source[vertOffsets[k]];
				}
			}
			polys.push_back(verts);
		}
		return polys;
	}

	std::size_t Scheme::polyCount() const
	{
		return offsets.size();
	}

}