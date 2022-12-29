#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/gl/canvas_gui_item.h>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/collections/SetNamer.hpp>
#include <cpputils/collections/Event.hpp>
#include <cinolib/color.h>

namespace HMP::Gui::Widgets
{

    class Debug final: public cinolib::SideBarItem, public cinolib::CanvasGuiItem, public cpputils::mixins::ReferenceClass
    {

    private:

        const Meshing::Mesher& m_mesher;
        cpputils::collections::SetNamer<const HMP::Dag::Node*>& m_dagNamer;
        VertEdit& m_vertEdit;

        Real m_eps{ 1e-9 };

        void selectCloseVerts();

    public:

        cinolib::Color nameColor{ cinolib::Color::YELLOW() };

        float fontSize{ 12.0f };
        float themeHue{ 32.0f };
        bool themeDark{ true };
        bool showElements{ false }, showVids{ false }, showEids{ false }, showFids{ false }, showPids{ false };

        cpputils::collections::Event<Debug, bool, float> onThemeChangeRequested;

        Debug(Meshing::Mesher& _mesher, cpputils::collections::SetNamer<const HMP::Dag::Node*>& _dagNamer, VertEdit& _vertEdit);

        void draw(const cinolib::GLcanvas& _canvas) override;

        void draw() override;

        void requestThemeUpdate();

    };

}