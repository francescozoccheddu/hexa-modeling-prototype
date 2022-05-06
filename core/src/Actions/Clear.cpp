#include <HMP/Actions/Clear.hpp>

#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Clear::Clear()
	{}

	void Clear::apply()
	{
		constexpr double cubeSize{ 1 };
		m_root = root();
		root() = new Dag::Element{};
		root()->vertices() = {
			Vec(-cubeSize,-cubeSize,-cubeSize), Vec(-cubeSize,-cubeSize, cubeSize), Vec(cubeSize,-cubeSize,cubeSize), Vec(cubeSize,-cubeSize,-cubeSize),
			Vec(-cubeSize,cubeSize,-cubeSize), Vec(-cubeSize,cubeSize, cubeSize), Vec(cubeSize,cubeSize,cubeSize), Vec(cubeSize,cubeSize,-cubeSize)
		};
		mesher().clear();
		mesher().add(*root());
	}

	void Clear::unapply()
	{
		if (root())
		{
			root()->children().detachAll(true);
			delete root();
		}
		root() = m_root;
		Meshing::Utils::addLeafs(mesher(), *root(), true);
	}

}