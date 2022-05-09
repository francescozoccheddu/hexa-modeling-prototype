#include <HMP/Actions/Load.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <algorithm>

namespace HMP::Actions
{

	Load::~Load()
	{
		if (m_otherRoot)
		{
			m_otherRoot->children().detachAll(true);
			delete m_otherRoot;
		}
	}

	void Load::apply()
	{
		std::swap(m_otherRoot, root());
		Meshing::Utils::addLeafs(mesher(), *root(), true);
		mesher().updateMesh();
	}

	void Load::unapply()
	{
		apply();
	}

	Load::Load(Dag::Element& _root)
		: m_otherRoot{ &_root }
	{
	}

}