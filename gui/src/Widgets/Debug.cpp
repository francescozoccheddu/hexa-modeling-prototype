#include <HMP/Gui/Widgets/Debug.hpp>

#include <imgui.h>
#include <cinolib/memory_usage.h>
#include <unordered_map>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <algorithm>
#include <string>
#include <cinolib/fonts/droid_sans.hpp>
#include <imgui_impl_opengl2.h>
#include <HMP/Gui/themer.hpp>

namespace HMP::Gui::Widgets
{

    Debug::Debug(Meshing::Mesher& _mesher, cpputils::collections::SetNamer<const HMP::Dag::Node*>& _dagNamer, VertEdit& _vertEdit)
        : cinolib::SideBarItem{ "Debug" }, m_mesher{ _mesher }, m_dagNamer{ _dagNamer }, m_vertEdit{ _vertEdit }
    {}

    void Debug::draw(const cinolib::GLcanvas& _canvas)
    {
        using namespace Utils::Drawing;
        const Meshing::Mesher::Mesh& mesh{ m_mesher.mesh() };
        ImDrawList& drawList{ *ImGui::GetWindowDrawList() };
        const float fontSize{ ImGui::GetFontSize() * namesFontScale * themer->ovScale };
        if (showVids)
        {
            const Id count{ std::min<Id>(mesh.num_verts(), 100) };
            for (Id vid{}; vid < count; vid++)
            {
                const Vec vert{ mesh.vert(vid) };
                text(drawList, std::to_string(vid).c_str(), project(_canvas, vert), fontSize, themer->ovHi);
            }
        }
        if (showEids)
        {
            const Id count{ std::min<Id>(mesh.num_edges(), 100) };
            for (Id eid{}; eid < count; eid++)
            {
                const Vec vert{ Meshing::Utils::centroid(Meshing::Utils::verts(mesh, Meshing::Utils::eidVids(mesh, eid))) };
                text(drawList, std::to_string(eid).c_str(), project(_canvas, vert), fontSize, themer->ovHi);
            }
        }
        if (showFids)
        {
            const Id count{ std::min<Id>(mesh.num_faces(), 100) };
            for (Id fid{}; fid < count; fid++)
            {
                const Vec vert{ mesh.face_centroid(fid) };
                text(drawList, std::to_string(fid).c_str(), project(_canvas, vert), fontSize, themer->ovHi);
            }
        }
        if (showPids)
        {
            const Id count{ std::min<Id>(mesh.num_polys(), 100) };
            for (Id pid{}; pid < count; pid++)
            {
                const Vec vert{ mesh.poly_centroid(pid) };
                text(drawList, std::to_string(pid).c_str(), project(_canvas, vert), fontSize, themer->ovHi);
            }
        }
        if (showElements)
        {
            const Id count{ std::min<Id>(mesh.num_polys(), 100) };
            for (Id pid{}; pid < count; pid++)
            {
                const Vec vert{ mesh.poly_centroid(pid) };
                text(drawList, m_dagNamer.nameOrUnknown(&m_mesher.element(pid)).c_str(), project(_canvas, vert), fontSize, themer->ovHi);
            }
        }
    }

    void Debug::draw()
    {
        if (ImGui::TreeNode("Names"))
        {
            ImGui::Spacing();
            ImGui::BeginTable("names", 3, ImGuiTableFlags_RowBg);
            ImGui::TableSetupColumn("doit", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("desc", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("reset", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextColumn();
            ImGui::Checkbox("##elements", &showElements);
            ImGui::TableNextColumn();
            ImGui::Text("Elements");
            ImGui::TableNextColumn();
            if (ImGui::Button("Reset"))
            {
                m_dagNamer.reset();
            }

            ImGui::TableNextColumn();
            ImGui::Checkbox("##pids", &showPids);
            ImGui::TableNextColumn();
            ImGui::Text("Pids");
            ImGui::TableNextColumn();

            ImGui::TableNextColumn();
            ImGui::Checkbox("##fids", &showFids);
            ImGui::TableNextColumn();
            ImGui::Text("Fids");
            ImGui::TableNextColumn();

            ImGui::TableNextColumn();
            ImGui::Checkbox("##eids", &showEids);
            ImGui::TableNextColumn();
            ImGui::Text("Eids");
            ImGui::TableNextColumn();

            ImGui::TableNextColumn();
            ImGui::Checkbox("##vids", &showVids);
            ImGui::TableNextColumn();
            ImGui::Text("Vids");
            ImGui::TableNextColumn();

            ImGui::EndTable();
            ImGui::Spacing();
            Utils::Controls::sliderPercentage("Font scale", namesFontScale, 0.5f, 2.0f);
            ImGui::Spacing();
            ImGui::TreePop();
        }
        // theme
        if (ImGui::TreeNode("Theme"))
        {
            ImGui::Spacing();
            if (ImGui::Checkbox("Dark", &themeDark))
            {
                updateTheme();
            }
            ImGui::SameLine();
            if (ImGui::SliderFloat("Hue", &themeHue, 0.0f, 360.0f, "%.0f deg", ImGuiSliderFlags_AlwaysClamp))
            {
                updateTheme();
            }
            ImGui::Spacing();
            if (Utils::Controls::sliderPercentage("Scale", themeScale, 0.5f, 2.0f))
            {
                updateTheme();
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset"))
            {
                themeScale = 1.0f;
                updateTheme();
            }
            ImGui::Spacing();
            ImGui::TreePop();
        }
        // verts
        if (ImGui::TreeNode("Tests"))
        {
            ImGui::Spacing();
            static constexpr double minEps{ 1e-9 }, maxEps{ 1e-3 };
            ImGui::SliderScalar("Epsilon", ImGuiDataType_Double, &testEps, &minEps, &maxEps, "%.3e", ImGuiSliderFlags_AlwaysClamp);
            ImGui::BeginTable("tests", 2, ImGuiTableFlags_RowBg);
            ImGui::TableSetupColumn("run", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("desc", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Run##closeverts"))
            {
                selectCloseVerts();
            }
            ImGui::TableNextColumn();
            ImGui::Text("Select close verts");
            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Run##crash"))
            {
                throw std::runtime_error{ "user requested test crash" };
            }
            ImGui::TableNextColumn();
            ImGui::TextColored(themer->sbWarn, "Throw an exception");
            ImGui::EndTable();
            ImGui::Spacing();
            ImGui::TreePop();
        }
        // debug
        if (ImGui::TreeNode("Stats"))
        {
            ImGui::Spacing();
            ImGui::BeginTable("stats", 2, ImGuiTableFlags_RowBg);
            ImGui::TableNextColumn(); ImGui::Text("Configuration");
#ifdef NDEBUG
            ImGui::TableNextColumn(); ImGui::Text("Release");
#else
            ImGui::TableNextColumn(); ImGui::Text("Debug");
#endif
            ImGui::TableNextColumn(); ImGui::Text("Version");
            ImGui::TableNextColumn(); ImGui::Text(HMP_VERSION);
            ImGui::TableNextColumn(); ImGui::Text("Build date");
            ImGui::TableNextColumn(); ImGui::Text(__DATE__);
            ImGui::TableNextColumn(); ImGui::Text("Build time");
            ImGui::TableNextColumn(); ImGui::Text(__TIME__);
            ImGui::TableNextColumn(); ImGui::Text("Compiler ID");
            ImGui::TableNextColumn(); ImGui::Text(HMP_GUI_COMPILER_ID);
            ImGui::TableNextColumn(); ImGui::Text("Memory usage");
            ImGui::TableNextColumn(); ImGui::Text("%dMB", static_cast<int>(cinolib::memory_usage_in_mega_bytes() + 0.5f));
            ImGui::TableNextColumn(); ImGui::Text("Poly count");
            ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(m_mesher.mesh().num_polys()));
            ImGui::TableNextColumn(); ImGui::Text("Face count");
            ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(m_mesher.mesh().num_faces()));
            ImGui::TableNextColumn(); ImGui::Text("Edge count");
            ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(m_mesher.mesh().num_edges()));
            ImGui::TableNextColumn(); ImGui::Text("Vert count");
            ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(m_mesher.mesh().num_verts()));
#ifndef NDEBUG
            ImGui::TableNextColumn(); ImGui::Text("Allocated node count");
            ImGui::TableNextColumn(); ImGui::Text("%u", static_cast<unsigned int>(Dag::Node::allocatedNodeCount()));
#endif
            ImGui::EndTable();
            ImGui::Spacing();
            ImGui::TreePop();
        }
    }

    void Debug::updateTheme() const
    {
        themer.setTheme(Utils::Theme::make(themeDark, themeHue, themeScale));
    }

    void Debug::selectCloseVerts()
    {
        const Meshing::Mesher::Mesh& mesh{ m_mesher.mesh() };
        std::map<Vec, Id, Meshing::Utils::VertComparer> vertMap{ {.eps = testEps} };
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

}