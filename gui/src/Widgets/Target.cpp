#include <HMP/Gui/Widgets/Target.hpp>

#include <cinolib/gl/file_dialog_open.h>
#include <cinolib/geometry/lerp.hpp>
#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <vector>
#include <stdexcept>
#include <filesystem>

namespace HMP::Gui::Widgets
{

	Target::Target(const Meshing::Mesher::Mesh& _sourceMesh) :
		cinolib::SideBarItem{ "Target mesh" },
		m_mesh{}, m_sourceMesh{ _sourceMesh },
		onProjectRequest{}, onMeshLoad{}, onMeshClear{}, onApplyTransformToSource{}, onVertsInterpolationChanged{},
		m_visible{ true }, m_faceColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.15f} }, m_edgeColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.4f} },
		m_transform{}, m_verts{}, m_vertInterpProgress{ 1.0 }, m_sliderVertInterpProgress{ 100.0f },
		m_projectLines{}, m_showProjectLines{},
		m_missingMeshFile{ false }
	{
		m_projectLines.set_color(cinolib::Color::hsv2rgb(0.0f, 1.0f, 1.0f));
		m_projectLines.set_thickness(1.0f);
		m_projectLines.set_cheap_rendering(true);
		m_projectLines.set_always_in_front(false);
		m_projectLines.show = false;
	}

	const std::vector<std::pair<Vec, Vec>>& Target::verts() const
	{
		return m_verts;
	}

	Real Target::vertInterpolationProgress() const
	{
		return m_vertInterpProgress;
	}

	void Target::interpolateVerts(Real _progress)
	{
		if (_progress != m_vertInterpProgress)
		{
			m_vertInterpProgress = _progress;
			m_sliderVertInterpProgress = static_cast<float>(_progress) * 100.0f;
			std::unordered_map<Id, Vec> move{};
			move.reserve(m_verts.size());
			Id vid{};
			for (const auto& [a, b] : m_verts)
			{
				move.insert({ vid++, cinolib::lerp1(std::array<Vec,2>{a, b}, _progress) });
			}
			onVertsInterpolationChanged(move);
		}
	}

	void Target::cancelVertInterpolation()
	{
		interpolateVerts(1.0);
	}

	bool Target::interpolatingVerts() const
	{
		return m_vertInterpProgress != 1.0;
	}

	void Target::showProjectLines(bool _visible)
	{
		m_projectLines.show = _visible;
	}

	void Target::setProjectLinesColor(const cinolib::Color& _color)
	{
		m_projectLines.set_color(_color);
	}

	void Target::setProjectLinesThickness(float _thickness)
	{
		m_projectLines.set_thickness(_thickness);
	}

	void Target::setProjectLinesAlwaysOnFront(bool _alwaysInFront)
	{
		m_projectLines.set_always_in_front(_alwaysInFront);
	}

	const cinolib::DrawableSegmentSoup& Target::projectLines() const
	{
		return m_projectLines;
	}

	void Target::ensureHasMesh() const
	{
		if (!m_mesh)
		{
			throw std::logic_error{ "no mesh" };
		}
	}

	const Meshing::Mesher::Mesh& Target::sourceMesh() const
	{
		return m_sourceMesh;
	}

	bool Target::hasMesh() const
	{
		return m_mesh;
	}

	const cinolib::DrawablePolygonmesh<>& Target::mesh() const
	{
		ensureHasMesh();
		return *m_mesh;
	}

	cinolib::Polygonmesh<> Target::meshForProjection() const
	{
		ensureHasMesh();
		cinolib::Polygonmesh<> mesh{ *m_mesh };
		for (Id vid{}; vid < mesh.num_verts(); vid++)
		{
			Vec& vert{ mesh.vert(vid) };
			vert = m_mesh->transform * vert;
		}
		return mesh;
	}

	void Target::show(bool _visible)
	{
		m_visible = _visible;
		if (hasMesh())
		{
			updateVisibility();
		}
	}

	bool Target::visible() const
	{
		return m_visible;
	}

	bool& Target::visible()
	{
		return m_visible;
	}

	cinolib::Color& Target::faceColor()
	{
		return m_faceColor;
	}

	const cinolib::Color& Target::faceColor() const
	{
		return m_faceColor;
	}

	cinolib::Color& Target::edgeColor()
	{
		return m_edgeColor;
	}

	const cinolib::Color& Target::edgeColor() const
	{
		return m_edgeColor;
	}

	const Utils::Transform& Target::transform() const
	{
		return m_transform;
	}

	Utils::Transform& Target::transform()
	{
		return m_transform;
	}

	void Target::identity(bool _origin, bool _translation, bool _rotation, bool _scale)
	{
		if (_origin)
		{
			m_transform.origin = Vec{};
		}
		if (_translation)
		{
			m_transform.translation = Vec{};
		}
		if (_rotation)
		{
			m_transform.rotation = Vec{};
		}
		if (_scale)
		{
			m_transform.scale = { 1.0 };
		}
		if (hasMesh())
		{
			updateTransform();
		}
	}

	void Target::fit(bool _origin, bool _translation, bool _scale)
	{
		ensureHasMesh();
		if (_origin)
		{
			m_transform.origin = m_mesh->bbox().center();
		}
		if (_translation)
		{
			m_transform.translation = -m_transform.origin + m_sourceMesh.bbox().center();
		}
		if (_scale)
		{
			m_transform.scale = { m_sourceMesh.bbox().diag() / m_mesh->bbox().diag() };
		}
		updateTransform();
	}

	void Target::updateTransform()
	{
		ensureHasMesh();
		m_mesh->transform = m_transform.matrix();
		onTransform();
	}

	void Target::updateVisibility()
	{
		ensureHasMesh();
		m_mesh->show_mesh(m_visible);
	}

	void Target::updateColor(bool _face, bool _edge)
	{
		ensureHasMesh();
		if (_face)
		{
			m_mesh->poly_set_color(m_faceColor);
		}
		if (_edge)
		{
			m_mesh->edge_set_color(m_edgeColor);
		}
	}

	bool Target::load(bool _keepTransform)
	{
		const std::string filename{ cinolib::file_dialog_open() };
		if (!filename.empty())
		{
			m_visible = true;
			load(filename, _keepTransform);
			return true;
		}
		return false;
	}

	void Target::load(const std::string& _filename, bool _keepTransform)
	{
		clearMesh();
		m_filename = _filename;
		if (std::filesystem::exists(_filename))
		{
			m_visible = true;
			m_mesh = new cinolib::DrawablePolygonmesh<>(m_filename.c_str());
			m_mesh->draw_back_faces = false;
			if (!_keepTransform)
			{
				fit();
			}
			else
			{
				updateTransform();
			}
			updateVisibility();
			updateColor();
			onMeshLoad();
		}
		else
		{
			m_missingMeshFile = true;
		}
	}

	void Target::clearMesh()
	{
		m_missingMeshFile = false;
		m_filename = "";
		if (m_mesh)
		{
			onMeshClear();
			delete m_mesh;
			m_mesh = nullptr;
		}
	}

	void Target::requestProjection()
	{
		ensureHasMesh();
		std::vector<std::pair<Vec, Vec>> verts{};
		verts.resize(m_sourceMesh.num_verts());
		for (std::size_t vid{}; vid < m_sourceMesh.num_verts(); vid++)
		{
			verts[vid].first = m_sourceMesh.vert(static_cast<Id>(vid));
		}
		onProjectRequest();
		m_vertInterpProgress = 1.0;
		m_sliderVertInterpProgress = 100.0f;
		m_projectLines.clear();
		m_projectLines.reserve(verts.size() * 2);
		for (std::size_t vid{}; vid < m_sourceMesh.num_verts(); vid++)
		{
			verts[vid].second = m_sourceMesh.vert(static_cast<Id>(vid));
			m_projectLines.push_seg(verts[vid].first, verts[vid].second);
		}
		m_verts.swap(verts);
		m_projectLines.update_bbox();
	}

	void Target::requestApplyTransformToSource()
	{
		ensureHasMesh();
		updateTransform();
		onApplyTransformToSource(m_mesh->transform.inverse());
		identity();
	}

	void Target::clearDebugInfo()
	{
		cancelVertInterpolation();
		m_projectLines.clear();
		m_verts.clear();
	}

	void Target::draw()
	{
		if (hasMesh())
		{
			{
				ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "%s", m_filename.c_str());
				if (ImGui::Button("Clear"))
				{
					clearMesh();
					return;
				}
				ImGui::SameLine();
				if (ImGui::Button("Replace"))
				{
					if (!load(true))
					{
						return;
					}
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Visible", &m_visible))
				{
					updateVisibility();
				}
			}
			ImGui::Spacing();
			{
				ImGui::PushID(0);
				ImGui::Text("Transform");
				ImGui::SameLine();
				if (ImGui::SmallButton("Identity"))
				{
					identity();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Fit"))
				{
					fit();
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(1);
				const float targetMeshSize{ static_cast<float>(m_mesh->bbox().diag()) * 2 };
				if (Utils::Controls::dragTranslationVec("Origin", m_transform.origin, targetMeshSize))
				{
					updateTransform();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Identity"))
				{
					identity(true, false, false, false);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Center"))
				{
					fit(true, false, false);
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(2);
				const float sourceMeshSize{ static_cast<float>(m_sourceMesh.bbox().diag()) * 2 };
				if (Utils::Controls::dragTranslationVec("Translation", m_transform.translation, sourceMeshSize))
				{
					updateTransform();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Identity"))
				{
					identity(false, true, false, false);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Fit"))
				{
					fit(false, true, false);
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(3);
				if (Utils::Controls::dragRotation("Rotation", m_transform.rotation))
				{
					updateTransform();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Identity"))
				{
					identity(false, false, true, false);
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(4);
				const Real sourceAndTargetMeshScaleRatio{ m_sourceMesh.bbox().diag() / m_mesh->bbox().diag() * 3 };
				Real scale{ m_transform.avgScale() };
				if (Utils::Controls::dragScale("Scale", scale, sourceAndTargetMeshScaleRatio))
				{
					m_transform.scale = { scale };
					updateTransform();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Identity"))
				{
					identity(false, false, false, true);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Fit"))
				{
					fit(false, false, true);
				}
				ImGui::PopID();
			}
			ImGui::Spacing();
			{
				Utils::Controls::colorButton("Face color", m_faceColor);
				ImGui::SameLine();
				if (ImGui::SmallButton("Apply##face_color"))
				{
					updateColor(true, false);
				}
				Utils::Controls::colorButton("Edge color", m_edgeColor);
				ImGui::SameLine();
				if (ImGui::SmallButton("Apply##edge_color"))
				{
					updateColor(false, true);
				}
			}
			ImGui::Spacing();
			if (ImGui::Button("Project"))
			{
				requestProjection();
			}
			ImGui::SameLine();
			if (ImGui::Button("Apply transform to source"))
			{
				requestApplyTransformToSource();
			}
			if (!m_projectLines.empty())
			{
				ImGui::Checkbox("Show lines", &m_projectLines.show);
				if (m_projectLines.show)
				{
					ImGui::SameLine();
					ImGui::Checkbox("On top", &m_projectLines.no_depth_test);
				}
			}
			if (!m_verts.empty())
			{
				ImGui::SliderFloat("Interpolate", &m_sliderVertInterpProgress, 0.0f, 100.0f, "%.2f%%", ImGuiSliderFlags_AlwaysClamp);
				Real desiredProgress{ static_cast<Real>(m_sliderVertInterpProgress / 100.0f) };
				if (!Utils::Transform::isNull(desiredProgress - m_vertInterpProgress))
				{
					ImGui::SameLine();
					if (ImGui::SmallButton("Apply"))
					{
						interpolateVerts(desiredProgress);
					}
				}
				if (interpolatingVerts() || m_sliderVertInterpProgress != 100.0f)
				{
					ImGui::SameLine();
					if (ImGui::SmallButton("Reset"))
					{
						m_sliderVertInterpProgress = 100.0f;
						cancelVertInterpolation();
					}
				}
			}
		}
		else
		{
			if (m_missingMeshFile)
			{
				ImGui::Text("Missing mesh file");
				ImGui::TextDisabled("%s", m_filename.c_str());
				if (ImGui::Button("Clear"))
				{
					clearMesh();
				}
				ImGui::SameLine();
				if (ImGui::Button("Replace"))
				{
					load(true);
				}
			}
			if (ImGui::Button("Load"))
			{
				load();
			}
		}
	}

	void Target::serialize(HMP::Utils::Serialization::Serializer& _serializer) const
	{
		_serializer << hasMesh();
		if (hasMesh())
		{
			_serializer << m_filename;
			_serializer << m_transform.origin;
			_serializer << m_transform.translation;
			_serializer << m_transform.scale;
			_serializer << m_transform.rotation;
		}
	}

	void Target::deserialize(HMP::Utils::Serialization::Deserializer& _deserializer)
	{
		clearMesh();
		if (_deserializer.get<bool>())
		{
			const std::string filename{ _deserializer.get<std::string>() };
			_deserializer >> m_transform.origin;
			_deserializer >> m_transform.translation;
			_deserializer >> m_transform.scale;
			_deserializer >> m_transform.rotation;
			load(filename, true);
		}
	}

}
