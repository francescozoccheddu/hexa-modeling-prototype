#include <HMP/Gui/Widgets/Target.hpp>

#include <cinolib/gl/file_dialog_open.h>
#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <vector>
#include <cassert>
#include <filesystem>

namespace HMP::Gui::Widgets
{

	Target::Target(const Meshing::Mesher::Mesh& _sourceMesh):
		cinolib::SideBarItem{ "Target mesh" },
		m_mesh{}, m_sourceMesh{ _sourceMesh },
		onApplyTransformToSource{},
		m_visible{ true },
#ifdef HMP_GUI_LIGHT_THEME
		m_faceColor{ cinolib::Color{0.0f,0.0f,0.0f, 0.1f} }, m_edgeColor{ cinolib::Color{0.0f,0.0f,0.0f, 0.3f} },
#else
		m_faceColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.1f} }, m_edgeColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.3f} },
#endif
		m_transform{},
		m_missingMeshFile{ false }
	{}

	const Meshing::Mesher::Mesh& Target::sourceMesh() const
	{
		return m_sourceMesh;
	}

	bool Target::hasMesh() const
	{
		return m_mesh.num_verts();
	}

	const cinolib::DrawablePolygonmesh<>& Target::meshForDisplay() const
	{
		return m_mesh;
	}

	cinolib::Polygonmesh<> Target::meshForProjection() const
	{
		assert(hasMesh());
		cinolib::Polygonmesh<> mesh{ m_mesh };
		for (Id vid{}; vid < mesh.num_verts(); vid++)
		{
			Vec& vert{ mesh.vert(vid) };
			vert = m_mesh.transform * vert;
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
		assert(hasMesh());
		if (_origin)
		{
			m_transform.origin = m_mesh.bbox().center();
		}
		if (_translation)
		{
			m_transform.translation = -m_transform.origin + m_sourceMesh.bbox().center();
		}
		if (_scale)
		{
			m_transform.scale = { m_sourceMesh.bbox().diag() / m_mesh.bbox().diag() };
		}
		updateTransform();
	}

	void Target::updateTransform()
	{
		assert(hasMesh());
		m_mesh.transform = m_transform.matrix();
		onMeshChanged();
	}

	void Target::updateVisibility()
	{
		assert(hasMesh());
		m_mesh.show_mesh(m_visible);
	}

	void Target::updateColor(bool _face, bool _edge)
	{
		assert(hasMesh());
		if (_face)
		{
			m_mesh.poly_set_color(m_faceColor);
		}
		if (_edge)
		{
			m_mesh.edge_set_color(m_edgeColor);
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
			m_mesh = cinolib::DrawablePolygonmesh<>{ m_filename.c_str() };
			m_edgesPainted.clear();
			m_edgesPainted.resize(toI(m_mesh.num_edges()), false);
			m_mesh.show_marked_edge(false);
			m_mesh.draw_back_faces = false;
			if (!_keepTransform)
			{
				m_transform = {};
				fit();
			}
			updateVisibility();
			updateColor();
			updateTransform();
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
		m_mesh.clear();
		m_mesh.updateGL();
		onMeshChanged();
	}

	void Target::requestApplyTransformToSource()
	{
		assert(hasMesh());
		updateTransform();
		onApplyTransformToSource(m_mesh.transform.inverse());
		identity();
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
				const float targetMeshSize{ static_cast<float>(m_mesh.bbox().diag()) * 2 };
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
				const Real sourceAndTargetMeshScaleRatio{ m_sourceMesh.bbox().diag() / m_mesh.bbox().diag() * 3 };
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
			if (ImGui::Button("Apply transform to source"))
			{
				requestApplyTransformToSource();
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

	void Target::updateEdgeColor(Id _eid, const cinolib::Color& _color)
	{
		if (m_mesh.edge_data(_eid).color != _color)
		{
			m_mesh.edge_data(_eid).color = _color;
			m_mesh.updateGL_mesh_e(_eid, _eid);
		}
	}

	void Target::paintEdge(Id _eid, const cinolib::Color& _color)
	{
		m_edgesPainted[toI(_eid)] = true;
		updateEdgeColor(_eid, _color);
	}

	void Target::unpaintEdge(Id _eid)
	{
		if (m_edgesPainted[_eid])
		{
			m_edgesPainted[toI(_eid)] = false;
			updateEdgeColor(_eid, m_edgeColor);
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
