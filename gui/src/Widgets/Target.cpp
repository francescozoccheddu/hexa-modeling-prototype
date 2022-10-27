#include <HMP/Gui/Widgets/Target.hpp>

#include <cinolib/gl/file_dialog_open.h>
#include <imgui.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/deg_rad.h>
#include <cmath>
#include <stdexcept>

namespace HMP::Gui::Widgets
{

	Target::Target(const Meshing::Mesher::Mesh& _sourceMesh)
		: m_mesh{}, m_sourceMesh{ _sourceMesh },
		onProjectRequest{}, onMeshLoad{}, onMeshClear{}, onApplyTransformToSource{},
		m_visible{ true }, m_faceColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.25f} }, m_edgeColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.75f} },
		m_transform{},
		cinolib::SideBarItem{ "Target mesh" }
	{}

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

	const cinolib::DrawableTrimesh<>& Target::mesh() const
	{
		ensureHasMesh();
		return *m_mesh;
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

	void Target::identity(bool _center, bool _rotation, bool _scale)
	{
		m_transform.origin = m_mesh ? m_mesh->bbox().center() : Vec{};
		if (_center)
		{
			m_transform.translation = -m_transform.origin;
		}
		if (_rotation)
		{
			m_transform.rotation = {};
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

	void Target::fit(bool _center, bool _scale)
	{
		ensureHasMesh();
		m_transform.origin = m_mesh->bbox().center();
		if (_center)
		{
			m_transform.translation = -m_mesh->bbox().center() + m_sourceMesh.bbox().center();
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

	bool Target::load()
	{
		const std::string filename{ cinolib::file_dialog_open() };
		if (!filename.empty())
		{
			m_visible = true;
			load(filename);
			return true;
		}
		return false;
	}

	void Target::load(const std::string& _filename)
	{
		clearMesh();
		m_visible = true;
		m_filename = _filename;
		m_mesh = new cinolib::DrawableTrimesh<>(m_filename.c_str());
		m_mesh->draw_back_faces = false;
		fit();
		updateVisibility();
		updateColor();
		onMeshLoad();
	}

	void Target::clearMesh()
	{
		if (m_mesh)
		{
			onMeshClear();
			delete m_mesh;
			m_mesh = nullptr;
			m_filename = "";
		}
	}

	void Target::requestProjection()
	{
		ensureHasMesh();
		onProjectRequest();
	}

	void Target::requestApplyTransformToSource()
	{
		ensureHasMesh();
		updateTransform();
		onApplyTransformToSource(m_mesh->transform.inverse());
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
				const float sourceMeshSize{ static_cast<float>(m_sourceMesh.bbox().diag()) };
				cinolib::vec3<float> xyz{ (m_transform.translation + m_mesh->bbox().center()).cast<float>() };
				if (ImGui::DragFloat3("Center", xyz.ptr(), sourceMeshSize / 200))
				{
					m_transform.translation = xyz.cast<Real>() - m_mesh->bbox().center();
					updateTransform();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Identity"))
				{
					identity(true, false, false);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Fit"))
				{
					fit(true, false);
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(2);
				cinolib::vec3<float> xyz{ m_transform.rotation.cast<float>() };
				if (ImGui::DragFloat3("Rotation", xyz.ptr(), 0.5, -360, 360, "%.1f deg"))
				{
					m_transform.rotation = Utils::Transform::wrapAngles(xyz.cast<Real>());
					updateTransform();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Identity"))
				{
					identity(false, true, false);
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(3);
				const float sourceAndTargetMeshScaleRatio{ static_cast<float>(m_sourceMesh.bbox().diag() / m_mesh->bbox().diag()) };
				float scale{ static_cast<float>((m_transform.scale.x() + m_transform.scale.y() + m_transform.scale.z()) / 3 * 100.0) };
				if (ImGui::DragFloat("Scale", &scale, sourceAndTargetMeshScaleRatio, sourceAndTargetMeshScaleRatio * 10, sourceAndTargetMeshScaleRatio * 1000, "%.2f%%"))
				{
					m_transform.scale = { scale / 100.0 };
					updateTransform();
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Identity"))
				{
					identity(false, false, true);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Fit"))
				{
					fit(false, true);
				}
				ImGui::PopID();
			}
			ImGui::Spacing();
			{
				const auto inputColor{ [this](cinolib::Color& _color, const std::string& _label) {
					ImGui::ColorEdit4(_label.c_str(), _color.rgba);
					ImGui::SameLine();
					return ImGui::SmallButton("Apply");
				} };
				ImGui::PushID(0);
				if (inputColor(m_faceColor, "Face color"))
				{
					updateColor(true, false);
				}
				ImGui::PopID();
				ImGui::PushID(1);
				if (inputColor(m_edgeColor, "Edge color"))
				{
					updateColor(false, true);
				}
				ImGui::PopID();
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
		}
		else
		{
			if (ImGui::Button("Load"))
			{
				load();
			}
		}
	}

}
