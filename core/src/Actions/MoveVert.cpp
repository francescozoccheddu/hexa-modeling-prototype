#include <HMP/Actions/MoveVert.hpp>

#include <HMP/grid.hpp>

namespace HMP::Actions
{

    MoveVert::MoveVert(const Vec& _oldPosition, const Vec& _newPosition) 
        : m_oldPosition(_oldPosition), m_newPosition(_newPosition) 
    {}

    void MoveVert::apply()
    {
        Grid& grid{ this->grid() };
        grid.vert(grid.getVert(m_oldPosition), m_newPosition);
        grid.mesh().updateGL();
    }

    void MoveVert::unapply()
    {
        Grid& grid{ this->grid() };
        grid.vert(grid.getVert(m_newPosition), m_oldPosition);
        grid.mesh().updateGL();
    }

}