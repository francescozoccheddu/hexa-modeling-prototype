#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <vector>
#include <cinolib/gl/FreeCamera.hpp>
#include <cinolib/color.h>
#include <string>
#include <optional>

namespace HMP::Gui::Widgets
{

    class Ae3d2ShapeExporter final: public cinolib::SideBarItem
    {

    private:

        struct Keyframe final
        {
            std::vector<QuadVerts> polygons{};
            cinolib::FreeCamera<Real> camera{};
        };

        const Meshing::Mesher::Mesh& m_mesh;
        const cinolib::FreeCamera<Real>& m_camera;
        std::vector<Keyframe> m_keyframes;
        std::optional<std::string> m_sampleError;

    public:

        Ae3d2ShapeExporter(const Meshing::Mesher::Mesh& _mesh, const cinolib::FreeCamera<Real>& _camera);

        bool requestExport() const;

        bool requestSample();

        void clear();

        I keyframeCount() const;

        bool empty() const;

        void draw() override final;

    };

}