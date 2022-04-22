#include <HMP/actions/MoveVert.hpp>

#include <HMP/grid.hpp>

namespace HMP::Actions
{

    MoveVert::MoveVert(const cinolib::vec3d& _oldPosition, const cinolib::vec3d& _newPosition) 
        : m_oldPosition(_oldPosition), m_newPosition(_newPosition) 
    {}

    void MoveVert::apply()
    {
        Grid& grid{ this->grid() };
        grid.vert(grid.getVert(m_oldPosition), m_newPosition);
        grid.update_mesh();
    }

    void MoveVert::unapply()
    {
        Grid& grid{ this->grid() };
        grid.vert(grid.getVert(m_newPosition), m_oldPosition);
        grid.update_mesh();
    }

}