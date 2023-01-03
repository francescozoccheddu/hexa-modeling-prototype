#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <string>
#include <chrono>

namespace HMP::Gui::Widgets
{

    class Pad final: public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
    {

    private:

        const Meshing::Mesher::Mesh& m_mesh;

    public:

        cpputils::collections::Event<Pad, Real, I, Real, Real> onPadRequested;

        Real length{ 0.0 };
        I smoothIterations{ 1 };
        Real smoothSurfVertWeight{ 1.0 };
        Real cornerShrinkFactor{ 0.5 };

        Pad(const Meshing::Mesher::Mesh& _mesh);

        void requestPad();

        void draw() override;

    };

}