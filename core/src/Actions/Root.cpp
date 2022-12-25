#include <HMP/Actions/Root.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <algorithm>

namespace HMP::Actions
{

	void Root::apply()
	{
		std::vector<Vec> oldVerts{ mesher().mesh().vector_verts() };
		std::swap(m_otherRoot, root());
		mesher().clear();
		Meshing::Utils::addVerts(mesher(), m_otherVerts);
		m_otherVerts.swap(oldVerts);
		Meshing::Utils::addLeafs(mesher(), *root());
		mesher().updateMesh();
	}

	void Root::unapply()
	{
		apply();
	}

	Root::Root(Dag::Element& _root, const std::vector<Vec>& _verts)
		: m_otherRoot{ &_root }, m_newRoot{ _root }, m_otherVerts{ _verts }, m_newVerts{ _verts }
	{}

	const Dag::Element& Root::newRoot() const
	{
		return m_newRoot;
	}

	const std::vector<Vec>& Root::newVerts() const
	{
		return m_otherVerts;
	}

}