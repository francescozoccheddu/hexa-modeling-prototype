#include <HMP/Actions/MoveVert.hpp>

#include <algorithm>

namespace HMP::Actions
{

	void MoveVert::apply()
	{
		Meshing::Mesher& mesher{ this->mesher() };
		const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
		const Id vid{ mesh.poly_vert_id(mesher.elementToPid(m_element), m_vertOffset) };
		const Vec oldPosition( mesh.vert(vid) );
		mesher.moveVert(vid, m_otherPosition);
		m_otherPosition = oldPosition;
	}

	void MoveVert::unapply()
	{
		apply();
	}

	MoveVert::MoveVert(Dag::Element& _element, Id _vertOffset, const Vec& _position)
		: m_element{ _element }, m_vertOffset{ _vertOffset }, m_otherPosition(_position)
	{}


}