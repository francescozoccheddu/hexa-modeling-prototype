#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <HMP/Meshing/types.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>

namespace HMP::Gui::Widgets
{

    class Smooth final: public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
    {


    public:

        cpputils::collections::Event<Smooth, I, I, Real> onSmoothRequested;

        I surfaceIterations{ 1 };
        I internalIterations{ 1 };
        Real surfVertWeight{ 1.0 };

        Smooth();

        void requestSmooth();

        void draw() override;

    };

}