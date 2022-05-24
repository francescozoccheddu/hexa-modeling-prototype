#include <HMP/Gui/Widgets/Target.hpp>

#include <cinolib/gl/file_dialog_open.h>
#include <Eigen/Core>
#include <imgui.h>
#include <cinolib/gl/glcanvas.h>
#include <cinolib/deg_rad.h>

#include <stdexcept>

namespace HMP::Gui::Widgets
{

	Target::Target(const Meshing::Mesher::Mesh& _sourceMesh)
		: m_mesh{}, m_sourceMesh{ _sourceMesh },
		m_onProjectRequest{}, m_onMeshLoad{}, m_onMeshClear{},
		m_visible{ true }, m_faceColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.25f} }, m_edgeColor{ cinolib::Color{1.0f,1.0f,1.0f, 0.75f} },
		m_rotation{ 0,0,0 }, m_center{ 0,0,0 }, m_scale{ 1 },
		cinolib::SideBarItem{ "Target mesh" }
	{}

	void Target::ensureHasMesh() const
	{
		if (!m_mesh)
		{
			throw std::logic_error{ "no mesh" };
		}
	}

	std::function<void(const Target&)>& Target::onProjectRequest()
	{
		return m_onProjectRequest;
	}

	std::function<void(const Target&)>& Target::onMeshLoad()
	{
		return m_onMeshLoad;
	}

	std::function<void(const Target&)>& Target::onMeshClear()
	{
		return m_onMeshClear;
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

	cinolib::vec3d& Target::rotation()
	{
		return m_rotation;
	}

	const cinolib::vec3d& Target::rotation() const
	{
		return m_rotation;
	}

	cinolib::vec3d& Target::center()
	{
		return m_center;
	}

	const cinolib::vec3d& Target::center() const
	{
		return m_center;
	}

	double& Target::scale()
	{
		return m_scale;
	}

	double Target::scale() const
	{
		return m_scale;
	}

	void Target::translate(const cinolib::vec3d& _offset)
	{
		m_center += _offset;
		if (hasMesh())
		{
			updateTransform();
		}
	}

	void Target::rotate(const cinolib::vec3d& _axis, double _angleDeg)
	{
		if (hasMesh())
		{
			updateTransform();
		}
	}

	void Target::scale(double _amount)
	{
		m_scale *= _amount;
		if (hasMesh())
		{
			updateTransform();
		}
	}

	void Target::identity(bool _center, bool _rotation, bool _scale)
	{
		if (_center)
		{
			m_center = m_mesh->bbox().center();
		}
		if (_rotation)
		{
			m_rotation = cinolib::vec3d::ZERO();
		}
		if (_scale)
		{
			m_scale = 1.0;
		}
		if (hasMesh())
		{
			updateTransform();
		}
	}

	void Target::fit(bool _fitTranslation, bool _fitScale)
	{
		ensureHasMesh();
		if (_fitTranslation)
		{
			m_center = m_sourceMesh.bbox().center();
		}
		if (_fitScale)
		{
			m_scale = m_sourceMesh.bbox().diag() / m_mesh->bbox().diag();
		}
		updateTransform();
	}

	void Target::updateTransform()
	{
		ensureHasMesh();
		const cinolib::mat3d rotation3(
			cinolib::mat3d::ROT_3D(cinolib::GLcanvas::world_right, cinolib::to_rad(m_rotation.x())) *
			cinolib::mat3d::ROT_3D(cinolib::GLcanvas::world_up, cinolib::to_rad(m_rotation.y())) *
			cinolib::mat3d::ROT_3D(cinolib::GLcanvas::world_forward, cinolib::to_rad(m_rotation.z()))
		);
		const cinolib::mat4d rotation{
			rotation3(0,0),	rotation3(0,1),	rotation3(0,2),	0,
			rotation3(1,0),	rotation3(1,1),	rotation3(1,2),	0,
			rotation3(2,0),	rotation3(2,1),	rotation3(2,2),	0,
			0,				0,				0,				1
		};
		const cinolib::mat4d scale(cinolib::mat4d::DIAG(cinolib::vec4d{ m_scale, m_scale, m_scale, 1.0 }));
		const cinolib::mat4d centerTranslation(cinolib::mat4d::TRANS(m_center));
		const cinolib::mat4d originTranslation(cinolib::mat4d::TRANS(-m_mesh->bbox().center()));
		m_mesh->transform = rotation * centerTranslation * scale * originTranslation;
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

	void Target::load(const std::string& _filename)
	{
		clearMesh();
		m_visible = true;
		m_filename = _filename;
		m_mesh = new cinolib::DrawableTrimesh<>(m_filename.c_str());
		fit();
		updateVisibility();
		updateColor();
		if (m_onMeshLoad)
		{
			m_onMeshLoad(*this);
		}
	}

	void Target::clearMesh()
	{
		if (m_mesh)
		{
			if (m_onMeshClear)
			{
				m_onMeshClear(*this);
			}
			delete m_mesh;
			m_mesh = nullptr;
			m_filename = "";
		}
	}

	void Target::requestProjection()
	{
		if (!m_onProjectRequest)
		{
			throw std::logic_error{ "no projection callback" };
		}
		ensureHasMesh();
		m_onProjectRequest(*this);
	}

	void Target::draw()
	{
		if (hasMesh())
		{
			{
				ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), m_filename.c_str() + m_filename.find_last_of("/\\") + 1);
				ImGui::SameLine();
				if (ImGui::Button("Clear"))
				{
					clearMesh();
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Visible", &m_visible))
				{
					updateVisibility();
				}
			}
			ImGui::Spacing();
			const auto inputVec{ [this](cinolib::vec3d& _vec, const std::string& _label) {
				float xyz[3]{ static_cast<float>(_vec.x()), static_cast<float>(_vec.y()), static_cast<float>(_vec.z())};
				if (ImGui::InputFloat3(_label.c_str(), xyz))
				{
					_vec = cinolib::vec3d{ xyz[0], xyz[1], xyz[2] };
					updateTransform();
				}
			} };
			{
				ImGui::PushID(0);
				ImGui::Text("Transform");
				ImGui::SameLine();
				if (ImGui::Button("Identity"))
				{
					identity();
				}
				ImGui::SameLine();
				if (ImGui::Button("Fit"))
				{
					fit();
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(1);
				inputVec(m_center, "Center");
				ImGui::SameLine();
				if (ImGui::Button("Identity"))
				{
					identity(true, false, false);
				}
				ImGui::SameLine();
				if (ImGui::Button("Fit"))
				{
					fit(true, false);
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(2);
				inputVec(m_rotation, "Rotation");
				ImGui::SameLine();
				if (ImGui::Button("Identity"))
				{
					identity(false, true, false);
				}
				ImGui::PopID();
			}
			{
				ImGui::PushID(3);
				ImGui::InputDouble("Scale", &m_scale);
				ImGui::SameLine();
				if (ImGui::Button("Identity"))
				{
					identity(false, false, true);
				}
				ImGui::SameLine();
				if (ImGui::Button("Fit"))
				{
					fit(false, true);
				}
				ImGui::PopID();
			}
			ImGui::Spacing();
			{
				const auto inputColor{ [this](cinolib::Color& _color, const std::string& _label) {
					float rgba[4]{_color.r, _color.g, _color.b, _color.a};
					if (ImGui::ColorEdit4(_label.c_str(), rgba))
					{
						_color = cinolib::Color{ rgba[0], rgba[1], rgba[2], rgba[3]};
					}
					ImGui::SameLine();
					if (ImGui::Button("Apply"))
					{
						return true;
					}
					return false;
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
		}
		else
		{
			if (ImGui::Button("Load"))
			{
				const std::string filename{ cinolib::file_dialog_open() };
				if (!filename.empty())
				{
					m_visible = true;
					load(filename);
				}
			}
		}
	}

}