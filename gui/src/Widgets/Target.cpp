#include <HMP/Gui/Widgets/Target.hpp>

#include <cinolib/gl/file_dialog_open.h>
#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <vector>
#include <cassert>
#include <filesystem>
#include <HMP/Gui/themer.hpp>

namespace HMP::Gui::Widgets
{

	Target::Target(const Meshing::Mesher::Mesh& _sourceMesh):
		cinolib::SideBarItem{ "Target mesh" },
		m_mesh{}, m_sourceMesh{ _sourceMesh },
		m_missingMeshFile{ false },
		faceColor{ themer->tgtFace }, edgeColor{ themer->tgtEdge },
		transform{},
		visible{ true },
		onMeshChanged{}, onApplyTransformToSource{}
	{
		themer.onThemeChange += [this]() {
			faceColor = themer->tgtFace;
			edgeColor = themer->tgtEdge;
			if (hasMesh())
			{
				updateColor();
			}
		};
	}

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

	void Target::identity(bool _origin, bool _translation, bool _rotation, bool _scale)
	{
		if (_origin)
		{
			transform.origin = Vec{};
		}
		if (_translation)
		{
			transform.translation = Vec{};
		}
		if (_rotation)
		{
			transform.rotation = Vec{};
		}
		if (_scale)
		{
			transform.scale = { 1.0 };
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
			transform.origin = m_mesh.bbox().center();
		}
		if (_translation)
		{
			transform.translation = -transform.origin + m_sourceMesh.bbox().center();
		}
		if (_scale)
		{
			transform.scale = { m_sourceMesh.bbox().diag() / m_mesh.bbox().diag() };
		}
		updateTransform();
	}

	void Target::updateTransform()
	{
		assert(hasMesh());
		m_mesh.transform = transform.matrix();
		onMeshChanged();
	}

	void Target::updateVisibility()
	{
		assert(hasMesh());
		m_mesh.show_mesh(visible);
	}

	void Target::updateColor(bool _face, bool _edge)
	{
		assert(hasMesh());
		if (_face && m_mesh.poly_data(0).color != faceColor)
		{
			m_mesh.poly_set_color(faceColor);
		}
		if (_edge && m_mesh.edge_data(0).color != edgeColor)
		{
			m_mesh.edge_set_color(edgeColor);
		}
	}

	bool Target::load(bool _keepTransform)
	{
		const std::string filename{ cinolib::file_dialog_open() };
		if (!filename.empty())
		{
			visible = true;
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
			visible = true;
			m_mesh = cinolib::DrawablePolygonmesh<>{ m_filename.c_str() };
			m_mesh.show_marked_edge(false);
			m_mesh.draw_back_faces = false;
			updateColor();
			updateVisibility();
			if (!_keepTransform)
			{
				transform = {};
				fit();
			}
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
				ImGui::TextColored(themer->sbOk, "%s", m_filename.c_str());
				ImGui::Spacing();
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
				if (ImGui::Checkbox("Visible", &visible))
				{
					updateVisibility();
				}
			}
			ImGui::Spacing();
			{
				ImGui::BeginTable("Transform", 3, ImGuiTableFlags_RowBg);
				ImGui::TableSetupColumn("drag", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("identity", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("fit", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableNextColumn();
				ImGui::Text("Transform");
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##global"))
				{
					identity();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Fit##global"))
				{
					fit();
				}
				ImGui::TableNextColumn();
				const float targetMeshSize{ static_cast<float>(m_mesh.bbox().diag()) * 2 };
				if (Utils::Controls::dragTranslationVec("Origin", transform.origin, targetMeshSize))
				{
					updateTransform();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##origin"))
				{
					identity(true, false, false, false);
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Fit##origin"))
				{
					fit(true, false, false);
				}
				ImGui::TableNextColumn();
				const float sourceMeshSize{ static_cast<float>(m_sourceMesh.bbox().diag()) * 2 };
				if (Utils::Controls::dragTranslationVec("Translation", transform.translation, sourceMeshSize))
				{
					updateTransform();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##translation"))
				{
					identity(false, true, false, false);
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Fit##translation"))
				{
					fit(false, true, false);
				}
				ImGui::TableNextColumn();
				if (Utils::Controls::dragRotation("Rotation", transform.rotation))
				{
					updateTransform();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##rotation"))
				{
					identity(false, false, true, false);
				}
				ImGui::TableNextColumn();
				ImGui::TableNextColumn();
				const Real sourceAndTargetMeshScaleRatio{ m_sourceMesh.bbox().diag() / m_mesh.bbox().diag() * 3 };
				Real scale{ transform.avgScale() };
				if (Utils::Controls::dragScale("Scale##scale", scale, sourceAndTargetMeshScaleRatio))
				{
					transform.scale = { scale };
					updateTransform();
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Identity##scale"))
				{
					identity(false, false, false, true);
				}
				ImGui::TableNextColumn();
				if (ImGui::Button("Fit##scale"))
				{
					fit(false, false, true);
				}
				ImGui::EndTable();
			}
			ImGui::Spacing();
			{
				ImGui::BeginTable("Color", 2, ImGuiTableFlags_RowBg);
				ImGui::TableSetupColumn("pick", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("apply", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableNextColumn();
				Utils::Controls::colorButton("Face color", faceColor);
				ImGui::TableNextColumn();
				if (ImGui::Button("Apply##face"))
				{
					updateColor(true, false);
				}
				ImGui::TableNextColumn();
				Utils::Controls::colorButton("Edge color", edgeColor);
				ImGui::TableNextColumn();
				if (ImGui::Button("Apply##edge"))
				{
					updateColor(false, true);
				}
				ImGui::EndTable();
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
				ImGui::TextColored(themer->sbErr, "Missing mesh file");
				ImGui::TextColored(themer->sbWarn, "%s", m_filename.c_str());
				ImGui::Spacing();
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
			_serializer << transform.origin;
			_serializer << transform.translation;
			_serializer << transform.scale;
			_serializer << transform.rotation;
		}
	}

	void Target::deserialize(HMP::Utils::Serialization::Deserializer& _deserializer)
	{
		clearMesh();
		if (_deserializer.get<bool>())
		{
			const std::string filename{ _deserializer.get<std::string>() };
			_deserializer >> transform.origin;
			_deserializer >> transform.translation;
			_deserializer >> transform.scale;
			_deserializer >> transform.rotation;
			load(filename, true);
		}
	}

}
