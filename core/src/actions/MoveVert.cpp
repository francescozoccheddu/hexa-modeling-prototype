#include <HMP/actions/MoveVert.hpp>

#include <HMP/grid.hpp>

namespace HMP::Actions
{

    MoveVert::MoveVert(unsigned int _vid, cinolib::vec3d _position) 
        : m_vid(_vid), m_position(_position) 
    {}

    void MoveVert::apply()
    {
        Grid& grid{ this->grid() };
        m_oldPosition = grid.mesh.vert(m_vid);
        grid.vert(m_vid, m_position);
        grid.update_mesh();
    }

    void MoveVert::unapply()
    {
        Grid& grid{ this->grid() };
        const unsigned int vid{ grid.getVert(m_position) };
        grid.vert(m_vid, m_oldPosition);
        grid.update_mesh();
    }

}