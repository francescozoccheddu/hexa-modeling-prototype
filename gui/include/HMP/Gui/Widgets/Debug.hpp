#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/SidebarWidget.hpp>
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

    class Debug final: public SidebarWidget
    {

    private:

        cinolib::DrawableSegmentSoup m_sectionSoup;

        unsigned int m_negJacTestRes{}, m_closeVertsTestRes{};

        Real sectionValue() const;

        bool sectionExports{ false };
        bool sectionExportsInv{ false };
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

        void selectCloseVerts();

        void selectNegJacobianHexes();

        void refineSingle();

        void exportTarget() const;

        void exportSource(bool _includeInterior) const;

        void updateSection();

        void drawCanvas() override;

        void drawSidebar() override;

        std::vector<const cinolib::DrawableObject*> additionalDrawables() const override;

        void attached() override;

    public:

        Debug();

        void updateTheme() const;

    };

}