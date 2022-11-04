#include <HMP/Gui/Widgets/Ae3d2ShapeExporter.hpp>

#include <cpputils/collections/conversions.hpp>
#include <cinolib/gl/file_dialog_save.h>
#include <algorithm>
#include <imgui.h>
#include <string>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <ctime>
#include <iomanip>

namespace HMP::Gui::Widgets
{

    struct JProp final
    {
        const char* key;
    };

    struct JVec final
    {
        const Vec& vec;
    };

    std::ostream& operator<<(std::ostream& _stream, JProp _prop)
    {
        return _stream << '"' << _prop.key << '"' << ':';
    }

    std::ostream& operator<<(std::ostream& _stream, JVec _vec)
    {
        return _stream
            << '['
            << _vec.vec.x() << ','
            << _vec.vec.y() << ','
            << _vec.vec.z()
            << ']';
    }

    Ae3d2ShapeExporter::Ae3d2ShapeExporter(const Meshing::Mesher::Mesh& _mesh, const cinolib::FreeCamera<Real>& _camera) :
        cinolib::SideBarItem{ "ae-3d2shape exporter" }, m_camera{ _camera }, m_mesh{ _mesh }, m_sampleError{},
        m_keyframes{}
    {}

    bool Ae3d2ShapeExporter::requestExport() const
    {
        static constexpr double c_keyframeDuration{ 1.0 };
        if (empty())
        {
            throw std::logic_error{ "empty" };
        }
        const std::string filename{ cinolib::file_dialog_save() };
        if (!filename.empty())
        {
            std::ofstream file{};
            file.open(filename);
            file << '{';
            {
                std::time_t time = std::time(nullptr);
                file
                    << JProp{ "name" }
                << '"' << "HMP export (" << std::put_time(std::localtime(&time), "%d-%m-%Y %H-%M-%S") << ')' << '"';
            }
            file << ',';
            {
                file << JProp{ "camera" } << '[';
                for (std::size_t k{}; k < m_keyframes.size(); k++)
                {
                    const Keyframe& keyframe{ m_keyframes[k] };
                    if (k > 0)
                    {
                        file << ',';
                    }
                    file
                        << '{'
                        << JProp{ "time" } << c_keyframeDuration * k << ','
                        << JProp{ "value" };
                    {
                        file << '{';
                        const cinolib::FreeCamera<Real>& camera{ keyframe.camera };
                        {
                            file
                                << JProp{ "view" } << '{'
                                << JProp{ "up" } << JVec{ camera.view.up } << ','
                                << JProp{ "eye" } << JVec{ camera.view.eye } << ','
                                << JProp{ "forward" } << JVec{ camera.view.forward } << '}';
                        }
                        file << ',';
                        {

                            file << JProp{ "projection" } << '{';
                            if (camera.projection.perspective)
                            {
                                file
                                    << JProp{ "kind" } << '"' << "perspective" << '"' << ','
                                    << JProp{ "fieldOfViewDegrees" } << camera.projection.verticalFieldOfView;
                            }
                            else
                            {
                                file
                                    << JProp{ "kind" } << '"' << "orthographic" << '"' << ','
                                    << JProp{ "scale" } << 1.0 / camera.projection.verticalFieldOfView;
                            }
                            file << '}';
                        }
                        file << '}';
                    }
                    file << '}';
                }
                file << ']';
            }
            file << ',';
            {
                file << JProp{ "polygons" } << '[';
                for (std::size_t k{}; k < m_keyframes.size(); k++)
                {
                    const Keyframe& keyframe{ m_keyframes[k] };
                    if (k > 0)
                    {
                        file << ',';
                    }
                    file
                        << '{'
                        << JProp{ "time" } << c_keyframeDuration * k << ','
                        << JProp{ "value" };
                    {
                        const std::vector<FaceVerts>& polygons{ keyframe.polygons };
                        file << '[';
                        bool firstPolygon{ true };
                        for (const FaceVerts& polygon : polygons)
                        {
                            if (!firstPolygon)
                            {
                                file << ',';
                            }
                            firstPolygon = false;
                            file << '[';
                            bool firstVert{ true };
                            for (const Vec& vert : polygon)
                            {
                                if (!firstVert)
                                {
                                    file << ',';
                                }
                                firstVert = false;
                                file << JVec{ vert };
                            }
                            file << ']';
                        }
                        file << ']';
                    }
                    file << '}';
                }
                file << ']';
            }
            file << ',';
            {
                file << JProp{ "size" } << '{'
                    << JProp{ "width" } << m_keyframes[0].camera.projection.aspectRatio << ','
                    << JProp{ "height" } << 1
                    << '}';
            }
            file << '}';
            file.close();
            return true;
        }
        return false;
    }

    bool  Ae3d2ShapeExporter::requestSample()
    {
        if (!m_keyframes.empty() && m_keyframes[0].camera.projection.perspective != m_camera.projection.perspective)
        {
            m_sampleError = "Camera projection kind changed";
            return false;
        }
        Keyframe keyframe{};
        keyframe.camera = m_camera;
        for (Id fid{}; fid < m_mesh.num_faces(); fid++)
        {
            Id pid;
            if (m_mesh.face_is_visible(fid, pid))
            {
                std::vector<Vec> verts{ m_mesh.face_verts(fid) };
                if (m_mesh.poly_face_is_CW(pid, fid))
                {
                    std::reverse(verts.begin(), verts.end());
                }
                keyframe.polygons.push_back(cpputils::collections::conversions::toArray<4>(verts));
            }
        }
        if (!m_keyframes.empty() && m_keyframes[0].polygons.size() != keyframe.polygons.size())
        {
            m_sampleError = "Number of polygons changed";
            return false;
        }
        m_sampleError = std::nullopt;
        m_keyframes.push_back(keyframe);
        return true;
    }

    void  Ae3d2ShapeExporter::clear()
    {
        m_keyframes.clear();
        m_sampleError = std::nullopt;
    }

    std::size_t Ae3d2ShapeExporter::keyframeCount() const
    {
        return m_keyframes.size();
    }

    bool Ae3d2ShapeExporter::empty() const
    {
        return m_keyframes.empty();
    }

    void Ae3d2ShapeExporter::draw()
    {
        if (ImGui::Button("Sample"))
        {
            requestSample();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            clear();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%d keyframes", static_cast<int>(keyframeCount()));
        if (m_sampleError)
        {
            ImGui::TextColored(ImVec4{ 1.0f,0.0f,0.0f,1.0f }, "%s", m_sampleError->c_str());
        }
        if (empty())
        {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Export"))
        {
            requestExport();
        }
        if (empty())
        {
            ImGui::EndDisabled();
        }
    }

}