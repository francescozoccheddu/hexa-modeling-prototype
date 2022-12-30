#include <HMP/Gui/Widgets/Save.hpp>

#include <cinolib/gl/file_dialog_save.h>
#include <cinolib/gl/file_dialog_open.h>
#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <HMP/Gui/themer.hpp>

namespace HMP::Gui::Widgets
{

    Save::Save(): cinolib::SideBarItem{ "Save" }, m_filename{}, m_loaded{ false }, onSave{}, onLoad{}, onExportMesh{}
    {}

    void Save::apply(bool _load, const std::string& _filename)
    {
        m_loaded = _load;
        m_filename = _filename;
        m_lastTime = std::chrono::steady_clock::now();
        if (_load)
        {
            onLoad(_filename);
        }
        else
        {
            onSave(_filename);
        }
    }

    const std::string& Save::filename() const
    {
        return m_filename;
    }

    void Save::requestSave()
    {
        if (m_filename.empty())
        {
            requestSaveNew();
        }
        else
        {
            apply(false, m_filename);
        }
    }

    void Save::requestSaveNew()
    {
        const std::string filename{ cinolib::file_dialog_save() };
        if (!filename.empty())
        {
            apply(false, filename);
        }
    }

    void Save::requestLoad()
    {
        const std::string filename{ cinolib::file_dialog_open() };
        if (!filename.empty())
        {
            apply(true, filename);
        }
    }

    void Save::requestLoad(const std::string& _filename)
    {
        apply(true, _filename);
    }

    void Save::requestExportMesh()
    {
        const std::string filename{ cinolib::file_dialog_save() };
        if (!filename.empty())
        {
            onExportMesh(filename);
        }
    }

    void Save::draw()
    {
        const ImVec4
            sbOk{ Utils::Controls::toImVec4(themer->sbOk) },
            sbWarn{ Utils::Controls::toImVec4(themer->sbWarn) };
        if (!m_filename.empty())
        {
            const int elapsedMins{ static_cast<int>(std::chrono::duration_cast<std::chrono::minutes>(std::chrono::steady_clock::now() - m_lastTime).count()) };
            const char* actionStr{ m_loaded ? "Loaded" : "Saved" };
            if (elapsedMins < 1)
            {
                ImGui::TextColored(sbOk, "%s less than a minute ago", actionStr);
            }
            else if (elapsedMins == 1)
            {
                ImGui::TextColored(sbOk, "%s a minute ago", actionStr);
            }
            else
            {
                ImGui::TextColored(elapsedMins > 5 ? sbWarn : sbOk, "%s %d minutes ago", actionStr, elapsedMins);
            }
            ImGui::Text("%s", m_filename.c_str());
            ImGui::Spacing();
            if (ImGui::Button("Save"))
            {
                requestSave();
            }
            ImGui::SameLine();
            if (ImGui::Button("Save new"))
            {
                requestSaveNew();
            }
        }
        else
        {
            ImGui::TextColored(sbWarn, "Not saved");
            ImGui::Spacing();
            if (ImGui::Button("Save"))
            {
                requestSaveNew();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            requestLoad();
        }
        ImGui::SameLine();
        if (ImGui::Button("Export mesh"))
        {
            requestExportMesh();
        }
    }

}