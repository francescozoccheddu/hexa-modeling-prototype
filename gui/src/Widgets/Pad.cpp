#include <HMP/Gui/Widgets/Pad.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>

namespace HMP::Gui::Widgets
{

    Pad::Pad(const Meshing::Mesher::Mesh& _mesh): cinolib::SideBarItem{ "Pad" }, m_mesh{ _mesh } {}

    void Pad::requestPad()
    {
        onPadRequested(length, smoothIterations, smoothSurfVertWeight, cornerShrinkFactor);
    }

    void Pad::draw()
    {
        Utils::Controls::sliderI("Smooth iterations", smoothIterations, 0, 20);
        Utils::Controls::sliderPercentage("Smooth surface weight", smoothSurfVertWeight, 0.5, 2.0);
        Utils::Controls::sliderPercentage("Corner shrink factor", cornerShrinkFactor);
        const Real meshSize{ m_mesh.bbox().diag() };
        Utils::Controls::sliderReal("Length", length, meshSize / 500.0, meshSize / 5.0);
        if (ImGui::Button("Pad"))
        {
            requestPad();
        }
    }

}