#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/gl/canvas_gui_item.h>
#include <cinolib/meshes/drawable_trimesh.h>
#include <HMP/Meshing/Mesher.hpp>
#include <functional>
#include <cinolib/geometry/aabb.h>
#include <cinolib/color.h>
#include <string>

namespace HMP::Gui::Widgets
{

	class Target final : public cinolib::SideBarItem
	{

	private:

		cinolib::DrawableTrimesh<>* m_mesh;
		const Meshing::Mesher::Mesh& m_sourceMesh;
		std::function<void(const Target&)> m_onProjectRequest, m_onMeshLoad, m_onMeshClear;
		std::function<void(const Target&, const Mat4&)> m_onApplyTransformToSource;
		bool m_visible;
		std::string m_filename;
		Vec m_rotation, m_center;
		double m_scale;
		cinolib::Color m_faceColor, m_edgeColor;

		void ensureHasMesh() const;

	public:

		Target(const Meshing::Mesher::Mesh& _sourceMesh);

		std::function<void(const Target&)>& onProjectRequest();
		std::function<void(const Target&)>& onMeshLoad();
		std::function<void(const Target&)>& onMeshClear();
		std::function<void(const Target&, const Mat4&)>& onApplyTransformToSource();

		const Meshing::Mesher::Mesh& sourceMesh() const;

		bool hasMesh() const;
		const cinolib::DrawableTrimesh<>& mesh() const;

		void show(bool _visible);
		bool visible() const;
		bool& visible();

		cinolib::Color& faceColor();
		const cinolib::Color& faceColor() const;

		cinolib::Color& edgeColor();
		const cinolib::Color& edgeColor() const;

		Vec& rotation();
		const Vec& rotation() const;

		Vec& center();
		const Vec& center() const;

		double& scale();
		double scale() const;

		void translate(const Vec& _offset);
		void rotate(const Vec& _axis, double _angleDeg);
		void scale(double _amount);

		void identity(bool _center = true, bool _rotation = true, bool _scale = true);
		void fit(bool _fitTranslation = true, bool _fitScale = true);

		void updateTransform();
		void updateVisibility();
		void updateColor(bool _face = true, bool _edge = true);

		void load(const std::string& _filename);
		void clearMesh();
		void requestProjection();
		void requestApplyTransformToSource();

		void draw() override;

	};

}