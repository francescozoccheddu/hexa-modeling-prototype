#include <HMP/Actions/MoveVert.hpp>

#include <HMP/Meshing/Mesher.hpp>

namespace HMP::Actions
{

    MoveVert::MoveVert(const Vec& _oldPosition, const Vec& _newPosition) 
        : m_oldPosition(_oldPosition), m_newPosition(_newPosition) 
    {}

    void MoveVert::apply()
    {
        Meshing::Mesher& mesher{ this->mesher() };
        mesher.moveVert(mesher.getVert(m_oldPosition), m_newPosition);
    }

    void MoveVert::unapply()
    {
        Meshing::Mesher& mesher{ this->mesher() };
        mesher.moveVert(mesher.getVert(m_newPosition), m_oldPosition);
    }

}