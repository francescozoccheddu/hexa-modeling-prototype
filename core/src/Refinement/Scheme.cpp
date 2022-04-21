#include <HMP/Refinement/Scheme.hpp>

namespace HMP::Refinement
{

	std::size_t Scheme::polyCount() const
	{
		return offsets.size();
	}

}