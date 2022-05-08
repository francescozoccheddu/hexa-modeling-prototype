#include <HMP/Actions/Clear.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <algorithm>

namespace HMP::Actions
{

	Clear::~Clear()
	{
		if (m_otherRoot)
		{
			m_otherRoot->children().detachAll(true);
			delete m_otherRoot;
		}
	}

	void Clear::apply()
	{
		std::swap(m_otherRoot, root());
		Meshing::Utils::addLeafs(mesher(), *root(), true);
		mesher().updateMesh();
	}

	void Clear::unapply()
	{
		apply();
	}

	Clear::Clear()
		: m_otherRoot{ new Dag::Element{} }
	{
		constexpr double cubeSize{ 1 };
		m_otherRoot->vertices() = {
			Vec(-cubeSize,-cubeSize,-cubeSize), Vec(-cubeSize,-cubeSize, cubeSize), Vec(cubeSize,-cubeSize,cubeSize), Vec(cubeSize,-cubeSize,-cubeSize),
			Vec(-cubeSize,cubeSize,-cubeSize), Vec(-cubeSize,cubeSize, cubeSize), Vec(cubeSize,cubeSize,cubeSize), Vec(cubeSize,cubeSize,-cubeSize)
		};
	}

}