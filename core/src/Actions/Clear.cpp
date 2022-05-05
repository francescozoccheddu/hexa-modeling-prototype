#include <HMP/Actions/Clear.hpp>

#include <HMP/grid.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Clear::Clear()
	{}

	void Clear::apply()
	{
		constexpr double cubeSize{ 1 };
		m_root = root();
		if (root())
		{
			root()->children().detachAll(true);
			delete root();
		}
		root() = new Dag::Element{};
		root()->vertices() = {
			Vec(-cubeSize,-cubeSize,-cubeSize), Vec(-cubeSize,-cubeSize, cubeSize), Vec(cubeSize,-cubeSize,cubeSize), Vec(cubeSize,-cubeSize,-cubeSize),
			Vec(-cubeSize,cubeSize,-cubeSize), Vec(-cubeSize,cubeSize, cubeSize), Vec(cubeSize,cubeSize,cubeSize), Vec(cubeSize,cubeSize,-cubeSize)
		};
		grid().replaceDag(*root());
	}

	void Clear::unapply()
	{
		if (root())
		{
			root()->children().detachAll(true);
			delete root();
		}
		root() = m_root;
		grid().replaceDag(*root());
	}

}