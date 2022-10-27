#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/meshes/drawable_trimesh.h>
#include <HMP/Meshing/Mesher.hpp>
#include <cinolib/geometry/aabb.h>
#include <cinolib/color.h>
#include <string>
#include <cpputils/collections/Event.hpp>
#include <HMP/Gui/Utils/Transform.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP::Gui::Widgets
{

	class Target final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		cinolib::DrawableTrimesh<>* m_mesh;
		const Meshing::Mesher::Mesh& m_sourceMesh;
		bool m_visible;
		std::string m_filename;
		Utils::Transform m_transform;
		cinolib::Color m_faceColor, m_edgeColor;

		void ensureHasMesh() const;

	public:

		Target(const Meshing::Mesher::Mesh& _sourceMesh);

		cpputils::collections::Event<Target> onProjectRequest, onMeshLoad, onMeshClear;
		cpputils::collections::Event<Target, const Mat4&> onApplyTransformToSource;

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

		Utils::Transform& transform();
		const Utils::Transform& transform() const;

		void identity(bool _center = true, bool _rotation = true, bool _scale = true);
		void fit(bool _center = true, bool _scale = true);

		void updateTransform();
		void updateVisibility();
		void updateColor(bool _face = true, bool _edge = true);

		bool load();
		void load(const std::string& _filename);
		void clearMesh();
		void requestProjection();
		void requestApplyTransformToSource();

		void draw() override;

	};

}