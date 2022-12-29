#include <HMP/Gui/Widgets/Debug.hpp>

#include <imgui.h>
#include <cinolib/memory_usage.h>
#include <unordered_map>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <algorithm>
#include <string>

namespace HMP::Gui::Widgets
{

    Debug::Debug(Meshing::Mesher& _mesher, cpputils::collections::SetNamer<const HMP::Dag::Node*>& _dagNamer, VertEdit& _vertEdit)
        : cinolib::SideBarItem{ "Debug" }, m_mesher{ _mesher }, m_dagNamer{ _dagNamer }, m_vertEdit{ _vertEdit }
    {}

    void Debug::selectCloseVerts()
    {
        const Meshing::Mesher::Mesh& mesh{ m_mesher.mesh() };
        std::map<Vec, Id, Meshing::Utils::VertComparer> vertMap{ {.eps = m_eps} };
        m_vertEdit.clear();
        for (Id vid{}; vid < mesh.num_verts(); vid++)
        {
            const Vec vert{ mesh.vert(vid) };
            const auto it{ vertMap.find(vert) };
            if (it != vertMap.end())
            {
                m_vertEdit.add(vid);
                m_vertEdit.add(it->second);
            }
            else
            {
                vertMap.insert(it, { vert, vid });
            }
        }
    }

    void Debug::draw(const cinolib::GLcanvas& _canvas)
    {
        using namespace Utils::Drawing;
        const Meshing::Mesher::Mesh& mesh{ m_mesher.mesh() };
        ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
        const ImU32 colorU32{ toU32(nameColor) };
        if (showVids)
        {
            const Id count{ std::min<Id>(mesh.num_verts(), 100) };
            for (Id vid{}; vid < count; vid++)
            {
                const Vec vert{ mesh.vert(vid) };
                text(drawList, std::to_string(vid).c_str(), project(_canvas, vert), fontSize, colorU32);
            }
        }
        if (showEids)
        {
            const Id count{ std::min<Id>(mesh.num_edges(), 100) };
            for (Id eid{}; eid < count; eid++)
            {
                const Vec vert{ Meshing::Utils::centroid(Meshing::Utils::verts(mesh, Meshing::Utils::eidVids(mesh, eid))) };
                text(drawList, std::to_string(eid).c_str(), project(_canvas, vert), fontSize, colorU32);
            }
        }
        if (showFids)
        {
            const Id count{ std::min<Id>(mesh.num_faces(), 100) };
            for (Id fid{}; fid < count; fid++)
            {
                const Vec vert{ mesh.face_centroid(fid) };
                text(drawList, std::to_string(fid).c_str(), project(_canvas, vert), fontSize, colorU32);
            }
        }
        if (showPids)
        {
            const Id count{ std::min<Id>(mesh.num_polys(), 100) };
            for (Id pid{}; pid < count; pid++)
            {
                const Vec vert{ mesh.poly_centroid(pid) };
                text(drawList, std::to_string(pid).c_str(), project(_canvas, vert), fontSize, colorU32);
            }
        }
        if (showElements)
        {
            const Id count{ std::min<Id>(mesh.num_polys(), 100) };
            for (Id pid{}; pid < count; pid++)
            {
                const Vec vert{ mesh.poly_centroid(pid) };
                text(drawList, m_dagNamer.nameOrUnknown(&m_mesher.element(pid)).c_str(), project(_canvas, vert), fontSize, colorU32);
            }
        }
    }

    void Debug::draw()
    {
        if (ImGui::TreeNode("Names"))
        {
            ImGui::Spacing();
            ImGui::Checkbox("Show elements", &showElements);
            ImGui::SameLine();
            if (ImGui::SmallButton("Reset"))
            {
                m_dagNamer.reset();
            }
            ImGui::Checkbox("Show pids", &showPids);
            ImGui::Checkbox("Show fids", &showFids);
            ImGui::Checkbox("Show eids", &showEids);
            ImGui::Checkbox("Show vids", &showVids);
            ImGui::Spacing();
            ImGui::TreePop();
        }
        // theme
        if (ImGui::TreeNode("Theme"))
        {
            ImGui::Spacing();
            if (ImGui::Checkbox("Dark", &themeDark))
            {
                requestThemeUpdate();
            }
            if (ImGui::SliderFloat("Hue", &themeHue, 0.0f, 360.0f, "%.0f deg", ImGuiSliderFlags_AlwaysClamp))
            {
                requestThemeUpdate();
            }
            ImGui::Spacing();
            ImGui::TreePop();
        }
        // verts
        if (ImGui::TreeNode("Tests"))
        {
            ImGui::Spacing();
            static constexpr double minEps{ 1e-9 }, maxEps{ 1e-3 };
            ImGui::SliderScalar("Epsilon", ImGuiDataType_Double, &m_eps, &minEps, &maxEps, "%.3e", ImGuiSliderFlags_AlwaysClamp);
            if (ImGui::SmallButton("Select close verts"))
            {
                selectCloseVerts();
            }
            if (ImGui::SmallButton("Crash me!"))
            {
                throw std::runtime_error{ "user requested crash" };
            }
            ImGui::Spacing();
            ImGui::TreePop();
        }
        // debug
        if (ImGui::TreeNode("Stats"))
        {
            ImGui::Spacing();
#ifdef NDEBUG
            ImGui::TextDisabled("Release build");
#else
            ImGui::TextDisabled("Debug build");
#endif
            ImGui::TextDisabled("Version: " HMP_VERSION);
            ImGui::TextDisabled("Date: " __DATE__);
            ImGui::TextDisabled("Time: " __TIME__);
            ImGui::TextDisabled("Compiler ID: " HMP_GUI_COMPILER_ID);
            ImGui::TextDisabled("Memory usage: %dMB", static_cast<int>(cinolib::memory_usage_in_mega_bytes() + 0.5f));
            ImGui::TextDisabled("Poly count: %u", static_cast<unsigned int>(m_mesher.mesh().num_polys()));
            ImGui::TextDisabled("Face count: %u", static_cast<unsigned int>(m_mesher.mesh().num_faces()));
            ImGui::TextDisabled("Edge count: %u", static_cast<unsigned int>(m_mesher.mesh().num_edges()));
            ImGui::TextDisabled("Vert count: %u", static_cast<unsigned int>(m_mesher.mesh().num_verts()));
#ifndef NDEBUG
            ImGui::TextDisabled("Allocated node count: %u", static_cast<unsigned int>(Dag::Node::allocatedNodeCount()));
#endif
            ImGui::Spacing();
            ImGui::TreePop();
        }
    }

    void Debug::requestThemeUpdate()
    {
        onThemeChangeRequested(themeDark, themeHue);
    }

}