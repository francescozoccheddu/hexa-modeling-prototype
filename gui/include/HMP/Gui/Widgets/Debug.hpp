#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/gl/canvas_gui_item.h>
#include <cinolib/drawable_segment_soup.h>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/collections/SetNamer.hpp>
#include <cpputils/collections/Event.hpp>
#include <cinolib/color.h>
#include <HMP/Refinement/Schemes.hpp>
#include <imgui.h>

namespace HMP::Gui::Widgets
{

    class Debug final: public cinolib::SideBarItem, public cinolib::CanvasGuiItem, public cpputils::mixins::ReferenceClass
    {

    private:

        const Meshing::Mesher& m_mesher;
        cpputils::collections::SetNamer<const HMP::Dag::Node*>& m_dagNamer;
        VertEdit& m_vertEdit;
        cinolib::DrawableSegmentSoup m_sectionSoup;

        unsigned int m_negJacTestRes{}, m_closeVertsTestRes{};

    public:

        Real testEps{ 1e-9 };
        float fontBuildSize{ 13.0f };
        float namesFontScale{ 1.0f };
        Real sectionFactor{ 0.0 };
        int sectionDim{ 1 };
        float sectionLineThickness{ 1.0f };
        float themeHue{ 32.0f };
        bool themeDark{ true };
        float themeScale{ 1.0f };
        I fi{}, fiVi{};
        Refinement::EScheme refineSingleScheme{ Refinement::EScheme::Test };
        bool showElements{ false }, showVids{ false }, showEids{ false }, showFids{ false }, showPids{ false };

        cpputils::collections::Event<Debug, Refinement::EScheme, I, I> onRefineSingleRequested;

        Debug(Meshing::Mesher& _mesher, cpputils::collections::SetNamer<const HMP::Dag::Node*>& _dagNamer, VertEdit& _vertEdit);

        const cinolib::DrawableSegmentSoup& sectionSoup() const;
        void updateSection();

        void draw(const cinolib::GLcanvas& _canvas) override;

        void draw() override;

        void updateTheme() const;

        void selectCloseVerts();
        void selectNegJacobianHexes();
        void refineSingle();

    };

}