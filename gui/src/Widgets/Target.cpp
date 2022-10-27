#include <HMP/Gui/Widgets/Target.hpp>

#include <cinolib/gl/file_dialog_open.h>
#include <imgui.h>
#include <HMP/Gui/Utils/Controls.hpp>
#include <vector>
#include <stdexcept>

namespace HMP::Gui::Widgets
{

	Target::Target(const Meshing::Mesher::Mesh& _sourceMesh) :
		cinolib::SideBarItem{ "Target mesh" },
		m_mesh{}, m_sourceMesh{ _sourceMesh },
		onProjectRequest{}, onMeshLoad{}, onMeshClear{}, onApplyTransformToSource{},
		m_visible{ true }, m_faceColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.25f} }, m_edgeColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.75f} },
		m_transform{},
		m_projectLines{}, m_showProjectLines{}
	{
		m_projectLines.set_color(cinolib::Color::hsv2rgb(0.0f, 1.0f, 1.0f));
		m_projectLines.set_thickness(1.0f);
		m_projectLines.set_cheap_rendering(true);
		m_projectLines.set_always_in_front(true);
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
		std::vector<Vec> fromVerts{};
		fromVerts.reserve(m_sourceMesh.num_verts());
		for (Id vid{}; vid < m_sourceMesh.num_verts(); vid++)
		{
			fromVerts.push_back(m_sourceMesh.vert(vid));
		}
		onProjectRequest();
		m_projectLines.clear();
		m_projectLines.reserve(fromVerts.size() * 2);
		for (Id vid{}; vid < m_sourceMesh.num_verts(); vid++)
		{
			m_projectLines.push_seg(fromVerts[static_cast<std::size_t>(vid)], m_sourceMesh.vert(vid));
		}
		m_projectLines.update_bbox();
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
				Vec center{ m_transform.translation + m_mesh->bbox().center() };
				if (Utils::Controls::dragTranslationVec("Center", center, sourceMeshSize))
				{
					m_transform.translation = center - m_mesh->bbox().center();
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
				if (Utils::Controls::dragRotation("Rotation", m_transform.rotation))
				{
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
				const Real sourceAndTargetMeshScaleRatio{ m_sourceMesh.bbox().diag() / m_mesh->bbox().diag() };
				Real scale{ m_transform.avgScale() };
				if (Utils::Controls::dragScale("Scale", scale, sourceAndTargetMeshScaleRatio))
				{
					m_transform.scale = { scale };
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
				if (Utils::Controls::colorButton("Face color", m_faceColor))
				{
					updateColor(true, false);
				}
				ImGui::SameLine();
				if (Utils::Controls::colorButton("Edge color", m_edgeColor))
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
				ImGui::SameLine();
				ImGui::Checkbox("Show lines", &m_projectLines.show);
				if (m_projectLines.show)
				{
					ImGui::SameLine();
					ImGui::Checkbox("On top", &m_projectLines.no_depth_test);
				}
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
