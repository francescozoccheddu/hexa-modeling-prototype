#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <string>
#include <chrono>

namespace HMP::Gui::Widgets
{

    class Save final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
    {

    private:

        std::string m_filename;
        std::chrono::time_point<std::chrono::steady_clock> m_lastTime;
        bool m_loaded;

        void apply(bool _load, const std::string& _filename);

    public:

        cpputils::collections::Event<Save, const std::string&> onSave, onLoad, onExportMesh;

        explicit Save();

        const std::string& filename() const;

        void requestSave();

        void requestSaveNew();

        void requestLoad();
        
        void requestLoad(const std::string& _filename);

        void requestExportMesh();

        void draw() override;

    };

}