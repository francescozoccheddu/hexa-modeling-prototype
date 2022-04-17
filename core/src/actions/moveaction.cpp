#include <hexa-modeling-prototype/actions/moveaction.hpp>

namespace HMP
{

    MoveAction::MoveAction(Grid& grid, unsigned int vid, cinolib::vec3d displacement) : grid(grid), vid(vid), displacement(displacement) {}

    void MoveAction::execute()
    {
        auto& mesh = grid.mesh;

        for (unsigned int pid : mesh.adj_v2p(vid))
        {

            unsigned int vert_offset = mesh.poly_vert_offset(pid, vid);
            auto id = mesh.poly_verts_id(pid, true);
            auto element = grid.vids2element()[id];

            elements.push_back(element);
            offsets.push_back(vert_offset);

            grid.op_tree.move(element, vert_offset, mesh.vert(vid) + displacement);
        }

        grid.move(vid, displacement);

    }

    void MoveAction::undo()
    {
        for (unsigned int i = 0; i < elements.size(); i++)
        {

            auto& el = elements[i];
            unsigned int off = offsets[i];

            grid.op_tree.move(el, off, -displacement);
        }

        unsigned int pid = grid.vids2pid(grid.element2vids()[elements.front()]);
        unsigned int vid = grid.mesh.poly_vert_id(pid, offsets.front());

        grid.move(vid, -displacement);
    }

}