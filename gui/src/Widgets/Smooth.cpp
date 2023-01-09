#include <HMP/Gui/Widgets/Smooth.hpp>

#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>

namespace HMP::Gui::Widgets
{

    Smooth::Smooth(): SidebarWidget{ "Smooth" } {}

    void Smooth::requestSmooth()
    {
        onSmoothRequested(surfaceIterations, internalIterations, surfVertWeight);
    }

    void Smooth::drawSidebar()
    {
        Utils::Controls::sliderI("Surface iterations", surfaceIterations, 0, 20);
        Utils::Controls::sliderI("Internal iterations", internalIterations, 0, 20);
        Utils::Controls::sliderPercentage("Smooth surface weight", surfVertWeight, 0.5, 2.0);
        if (ImGui::Button("Smooth"))
        {
            requestSmooth();
        }
    }

}