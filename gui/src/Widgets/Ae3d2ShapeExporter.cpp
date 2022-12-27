#include <HMP/Gui/Widgets/Ae3d2ShapeExporter.hpp>

#include <HMP/Gui/Utils/Controls.hpp>
#include <cinolib/gl/file_dialog_save.h>
#include <algorithm>
#include <imgui.h>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cassert>
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

	Ae3d2ShapeExporter::Ae3d2ShapeExporter(const Meshing::Mesher::Mesh& _mesh, const cinolib::FreeCamera<Real>& _camera):
		cinolib::SideBarItem{ "ae-3d2shape exporter" }, m_mesh{ _mesh }, m_camera{ _camera }, m_keyframes{}, m_sampleError{}
	{}

	bool Ae3d2ShapeExporter::requestExport() const
	{
		static constexpr double c_keyframeDuration{ 1.0 };
		assert(!empty());
		const std::string filename{ cinolib::file_dialog_save() };
		if (!filename.empty())
		{
			std::ofstream file{};
			file.open(filename);
			file << '{'; // project
			{
				std::time_t time = std::time(nullptr);
				file << JProp{ "name" } << '"' << "HMP export (" << std::put_time(std::localtime(&time), "%d-%m-%Y %H-%M-%S") << ')' << '"'; // project.name
			}
			file << ','; // project
			{
				file << JProp{ "keyframes" } << '['; // project.keyframes
				for (I k{}; k < keyframeCount(); k++)
				{
					if (k > 0)
					{
						file << ','; // project.keyframes
					}
					file
						<< '{'
						<< JProp{ "time" } << static_cast<double>(k) * c_keyframeDuration << ','
						<< JProp{ "scene" } << '{'; // project.keyframes[k]
					{
						const cinolib::FreeCamera<Real>& camera{ m_keyframes[k].camera };
						file << JProp{ "camera" } << '{'; // project.keyframes[k].scene.camera
						{
							file
								<< JProp{ "view" } << '{'
								<< JProp{ "up" } << JVec{ camera.view.up } << ','
								<< JProp{ "eye" } << JVec{ camera.view.eye } << ','
								<< JProp{ "forward" } << JVec{ camera.view.forward } << '}'; // project.keyframes[k].scene.camera.view
						}
						file << ','; // project.keyframes[k].scene.camera
						{
							file << JProp{ "projection" } << '{';  // project.keyframes[k].scene.camera.projection
							if (camera.projection.perspective)
							{
								file
									<< JProp{ "kind" } << '"' << "perspective" << '"' << ','
									<< JProp{ "fov" } << camera.projection.verticalFieldOfView;  // project.keyframes[k].scene.camera.projection
							}
							else
							{
								file
									<< JProp{ "kind" } << '"' << "orthographic" << '"' << ','
									<< JProp{ "scale" } << 1.0 / camera.projection.verticalFieldOfView;  // project.keyframes[k].scene.camera.projection
							}
							file << '}'; // project.keyframes[k].scene.camera.projection
						}
						file << '}'; // project.keyframes[k].scene.camera
					}
					file << ','; // project.keyframes[k].scene
					{
						const cinolib::FreeCamera<Real>& camera{ m_keyframes[k].camera };
						file << JProp{ "lights" } << '['
							<< '{'
							<< JProp{ "kind" } << '"' << "directional" << '"' << ','
							<< JProp{ "direction" } << JVec{ camera.view.forward }
						<< '}' << ']'; // project.keyframes[k].scene.lights
					}
					file << ','; // project.keyframes[k].scene
					{
						file << JProp{ "polygons" } << '['; // project.keyframes[k].polygons
						bool firstPolygon{ true };
						for (const QuadVerts& face : m_keyframes[k].polygons)
						{
							if (!firstPolygon)
							{
								file << ','; // project.keyframes[k].scene.polygons
							}
							firstPolygon = false;
							file << '{' << JProp{ "vertices" } << '[';
							bool firstVert{ true };
							for (const Vec& vert : face)
							{
								if (!firstVert)
								{
									file << ','; // project.keyframes[k].scene.polygons[p].vertices
								}
								firstVert = false;
								file << JVec{ vert };
							}
							file << ']'; // project.keyframes[k].polygons[p].vertices
							file << '}'; // project.keyframes[k].polygons[p]
						}
						file << ']'; // project.keyframes[k].scene.polygons
					}
					file << '}'; // project.keyframes[k].scene
					file << '}'; // project.keyframe[k]
				}
				file << ']'; // project.keyframes
			}
			file << ','; // project
			{
				file << JProp{ "frameSize" } << '{'
					<< JProp{ "width" } << m_keyframes[0].camera.projection.aspectRatio << ','
					<< JProp{ "height" } << 1
					<< '}'; // project.frameSize
			}
			file << '}'; // project
			file.close();
			return true;
		}
		return false;
	}

	bool Ae3d2ShapeExporter::requestSample()
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
				keyframe.polygons.push_back(cpputils::range::of(verts).toArray<4>());
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

	void Ae3d2ShapeExporter::clear()
	{
		m_keyframes.clear();
		m_sampleError = std::nullopt;
	}

	I Ae3d2ShapeExporter::keyframeCount() const
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
		if (Utils::Controls::disabledButton("Export", !empty()))
		{
			requestExport();
		}
	}

}