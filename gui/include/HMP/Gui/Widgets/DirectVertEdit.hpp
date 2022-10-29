#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <cinolib/gl/canvas_gui_item.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/color.h>

namespace HMP::Gui::Widgets
{

    class DirectVertEdit final : public cinolib::CanvasGuiItem, public cpputils::mixins::ReferenceClass
    {

    public:

        enum class EKind
        {
            Translation, Rotation, Scale
        };

    private:

        static constexpr cinolib::Color c_mutedColor{ cinolib::Color::hsv2rgb(0.0f, 0.0f, 1.0f, 0.2f) };
        static constexpr cinolib::Color c_color{ cinolib::Color::hsv2rgb(0.1f, 0.75f, 1.0f, 0.7f) };
        static constexpr float c_lineThickness{ 1.0f };
        static constexpr float c_textSize{ 20.0f };
        static constexpr Vec2 c_textMargin{ 10.0f };

        VertEdit& m_vertEdit;
        const cinolib::GLcanvas& m_canvas;
        bool m_pending;
        bool m_onX, m_onY, m_onZ;
        EKind m_kind;
        Vec2 m_start;
        Vec2 m_centroid;
        Vec2 m_mouse;

        void ensurePending() const;

    public:

        cpputils::collections::Event<DirectVertEdit> onPendingChanged;

        DirectVertEdit(VertEdit& _vertEdit, const cinolib::GLcanvas& _canvas);

        void request(EKind _kind, const Vec2& _mouse);

        void update(const Vec2& _mouse, bool _onX = false, bool _onY = false, bool _onZ = false);

        void cancel();

        void apply();

        bool pending() const;

        EKind kind() const;

        bool locked() const;

        bool onX() const;

        bool onY() const;

        bool onZ() const;

        void draw() override final;

    };

}