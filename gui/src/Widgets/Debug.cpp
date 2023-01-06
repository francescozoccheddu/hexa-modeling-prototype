#include <HMP/Gui/Widgets/Debug.hpp>

#include <imgui.h>
#include <cinolib/memory_usage.h>
#include <cpputils/unreachable.hpp>
#include <unordered_map>
#include <unordered_set>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Gui/Utils/Drawing.hpp>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/Utils/FilePicking.hpp>
#include <algorithm>
#include <string>
#include <cinolib/fonts/droid_sans.hpp>
#include <imgui_impl_opengl2.h>
#include <HMP/Gui/themer.hpp>
#include <cinolib/quality_hex.h>
#include <cinolib/meshes/polygonmesh.h>

namespace HMP::Gui::Widgets
{

    Debug::Debug(Meshing::Mesher& _mesher, cpputils::collections::SetNamer<const HMP::Dag::Node*>& _dagNamer, VertEdit& _vertEditWidget, const Target& _targetWidget)
        : cinolib::SideBarItem{ "Debug" }, m_mesher{ _mesher }, m_dagNamer{ _dagNamer }, m_vertEditWidget{ _vertEditWidget }, m_targetWidget{ _targetWidget }, m_sectionSoup{}
    {
        m_sectionSoup.set_color(Utils::Drawing::toColor(themer->ovHi));
        m_sectionSoup.set_thickness(sectionLineThickness* themer->ovScale);
        themer.onThemeChange += [this]() {
            m_sectionSoup.set_color(Utils::Drawing::toColor(themer->ovHi));
            m_sectionSoup.set_thickness(sectionLineThickness * themer->ovScale);
        };
        m_sectionSoup.show = true;
        m_sectionSoup.use_gl_lines = true;
        m_sectionSoup.no_depth_test = true;
        _mesher.onUpdated += [this]() {
            updateSection();
        };
    }

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

    void Debug::selectNegJacobianHexes()
    {
        m_vertEditWidget.clear();
        m_negJacTestRes = 0;
        for (Id pid{}; pid < m_mesher.mesh().num_polys(); pid++)
        {
            if (m_mesher.shown(pid))
            {
                const std::vector<Vec>& verts{ m_mesher.mesh().poly_verts(pid) };
                if (cinolib::hex_scaled_jacobian(verts[0], verts[1], verts[2], verts[3], verts[4], verts[5], verts[6], verts[7]) < 0.0)
                {
                    m_vertEditWidget.add(m_mesher.mesh().adj_p2v(pid));
                    m_negJacTestRes++;
                }
            }
        }
    }

    void Debug::updateSection()
    {
        m_sectionSoup.clear();
        if (sectionFactor <= 0.0)
        {
            return;
        }
        const Id dim{ static_cast<Id>(sectionDim) };
        const Meshing::Mesher::Mesh& mesh{ m_mesher.mesh() };
        const Real v{ mesh.bbox().delta()[dim] * sectionFactor + mesh.bbox().min[dim] };
        std::vector<Id> pids;
        for (Id pid{}; pid < mesh.num_polys(); ++pid)
        {
            if (m_mesher.shown(pid))
            {
                const cinolib::AABB& aabb{ mesh.poly_aabb(pid) };
                if (aabb.min[dim] < v && aabb.max[dim] >= v)
                {
                    pids.push_back(pid);
                }
            }
        }
        std::unordered_set<Id> eids;
        for (const Id pid : pids)
        {
            for (const Id adjPid : mesh.adj_p2p(pid))
            {
                if (m_mesher.shown(adjPid))
                {
                    const cinolib::AABB& aabb{ mesh.poly_aabb(adjPid) };
                    if (aabb.max[dim] < v)
                    {
                        const Id fid{ static_cast<Id>(mesh.poly_shared_face(pid, adjPid)) };
                        for (const Id eid : mesh.adj_f2e(fid))
                        {
                            eids.insert(eid);
                        }
                    }
                }
            }
        }
        m_sectionSoup.reserve(eids.size() * 2);
        for (const Id eid : eids)
        {
            m_sectionSoup.push_seg(
                mesh.edge_vert(eid, 0),
                mesh.edge_vert(eid, 1)
            );
        }
    }

    const cinolib::DrawableSegmentSoup& Debug::sectionSoup() const
    {
        return m_sectionSoup;
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
        // Export
        if (ImGui::TreeNode("Export"))
        {
            ImGui::Spacing();
            ImGui::BeginTable("names", 2, ImGuiTableFlags_RowBg);
            ImGui::TableSetupColumn("doit", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("desc", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Export##sourcesurf"))
            {
                exportSource(false);
            }
            ImGui::TableNextColumn();
            ImGui::Text("Source surface");

            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Export##sourcevol"))
            {
                exportSource(true);
            }
            ImGui::TableNextColumn();
            ImGui::Text("Source surface and interior");

            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Export##target"))
            {
                exportTarget();
            }
            ImGui::TableNextColumn();
            ImGui::Text("Target surface");

            ImGui::Spacing();
            ImGui::EndTable();
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

            ImGui::BeginTable("tests", 3, ImGuiTableFlags_RowBg);
            ImGui::TableSetupColumn("run", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("desc", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("result", ImGuiTableColumnFlags_WidthFixed);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Run##closeverts"))
            {
                selectCloseVerts();
            }
            ImGui::TableNextColumn();
            ImGui::Text("Select close verts");
            ImGui::TableNextColumn();
            ImGui::Text("%u", m_closeVertsTestRes);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Run##jacobian"))
            {
                selectNegJacobianHexes();
            }
            ImGui::TableNextColumn();
            ImGui::Text("Select negative jacobian hexes");
            ImGui::TableNextColumn();
            ImGui::Text("%u", m_negJacTestRes);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (Utils::Controls::disabledSmallButton("Run##refinesingle", m_mesher.mesh().num_polys() == 1))
            {
                refineSingle();
            }
            ImGui::TableNextColumn();
            ImGui::Text("Refine single");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::SmallButton("Run##crash"))
            {
                throw std::runtime_error{ "user requested test crash" };
            }
            ImGui::TableNextColumn();
            ImGui::TextColored(themer->sbWarn, "Throw an exception");

            ImGui::EndTable();

            ImGui::Spacing();

            if (ImGui::TreeNode("Options"))
            {
                ImGui::Spacing();
                static constexpr double minEps{ 1e-9 }, maxEps{ 1e-3 };
                ImGui::SliderScalar("Epsilon", ImGuiDataType_Double, &testEps, &minEps, &maxEps, "%.3e", ImGuiSliderFlags_AlwaysClamp);
                Utils::Controls::combo<Refinement::EScheme>("Refinement scheme", refineSingleScheme, { "Subdivide3x3", "AdapterFaceSubdivide3x3", "Adapter2FacesSubdivide3x3", "AdapterEdgeSubdivide3x3", "Inset", "Subdivide2x2", "Test" });
                Utils::Controls::sliderI("Fi", fi, 0, 5);
                Utils::Controls::sliderI("Fi vi", fiVi, 0, 3);
                ImGui::Spacing();
                ImGui::TreePop();
            }

            ImGui::Spacing();
            ImGui::TreePop();
        }
        // section
        if (ImGui::TreeNode("Section"))
        {
            ImGui::Spacing();
            if (Utils::Controls::sliderPercentage("Factor", sectionFactor))
            {
                updateSection();
            }
            if (ImGui::Combo("Dimension", &sectionDim, "X\0Y\0Z\0"))
            {
                updateSection();
            }
            if (Utils::Controls::disabledButton("Clear", sectionFactor > 0.0))
            {
                sectionFactor = 0.0;
                updateSection();
            }
            ImGui::Spacing();
            ImGui::TreePop();
        }
        // stats
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
            ImGui::TableNextColumn(); ImGui::Text("Aggressive debug");
#ifdef HMP_AGGRESSIVE_DEBUG
            ImGui::TableNextColumn(); ImGui::Text("Enabled");
#else
            ImGui::TableNextColumn(); ImGui::Text("Disabled");
#endif
            ImGui::TableNextColumn(); ImGui::Text("Dag viewer");
#ifdef HMP_GUI_ENABLE_DAG_VIEWER
            ImGui::TableNextColumn(); ImGui::Text("Enabled");
#else
            ImGui::TableNextColumn(); ImGui::Text("Disabled");
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
        m_vertEditWidget.clear();
        for (Id vid{}; vid < mesh.num_verts(); vid++)
        {
            const Vec vert{ mesh.vert(vid) };
            const auto it{ vertMap.find(vert) };
            if (it != vertMap.end())
            {
                m_vertEditWidget.add(vid);
                m_vertEditWidget.add(it->second);
            }
            else
            {
                vertMap.insert(it, { vert, vid });
            }
        }
        m_closeVertsTestRes = static_cast<unsigned int>(m_vertEditWidget.vids().size());
    }

    void Debug::refineSingle()
    {
        if (m_mesher.mesh().num_polys() == 1 && m_mesher.shown(0))
        {
            onRefineSingleRequested(refineSingleScheme, fi, Meshing::Utils::hexFiVis[fi][fiVi]);
        }
    }

    void Debug::exportTarget() const
    {
        const std::optional<std::string> filename{ Utils::FilePicking::save("obj")};
        if (filename)
        {
            m_targetWidget.meshForProjection().save(filename->c_str());
        }
    }

    void Debug::exportSource(bool _includeInterior) const
    {
        const std::optional<std::string> filename{ Utils::FilePicking::save("obj")};
        if (!filename)
        {
            return;
        }
        const Meshing::Mesher::Mesh& vol{ m_mesher.mesh() };
        cinolib::Polygonmesh<> srf{};
        std::vector<Id> vidsMap(toI(vol.num_verts()), noId);
        std::vector<Id> fidVids{};
        fidVids.reserve(4);
        for (Id fid{}; fid < vol.num_faces(); fid++)
        {
            bool visible{ false };
            for (const Id adjPid : vol.adj_f2p(fid))
            {
                if (m_mesher.shown(adjPid))
                {
                    if (_includeInterior)
                    {
                        visible = true;
                        break;
                    }
                    else
                    {
                        if (visible)
                        {
                            visible = false;
                            break;
                        }
                        visible = true;
                    }
                }
            }
            if (visible)
            {
                fidVids.clear();
                for (const Id vid : vol.adj_f2v(fid))
                {
                    const I vi{ toI(vid) };
                    if (vidsMap[vi] == noId)
                    {
                        vidsMap[vi] = srf.num_verts();
                        srf.vert_add(vol.vert(vid));
                    }
                    fidVids.push_back(vidsMap[vi]);
                }
                srf.poly_add(fidVids);
            }
        }
        srf.update_bbox();
        srf.update_normals();
        srf.update_p_tessellations();
        srf.save(filename->c_str());
    }    


}