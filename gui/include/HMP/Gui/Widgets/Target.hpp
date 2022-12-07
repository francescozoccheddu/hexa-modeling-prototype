#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/meshes/drawable_polygonmesh.h>
#include <cinolib/meshes/polygonmesh.h>
#include <HMP/Meshing/Mesher.hpp>
#include <cinolib/color.h>
#include <string>
#include <cpputils/collections/Event.hpp>
#include <HMP/Gui/Utils/Transform.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Utils/Serialization.hpp>
#include <vector>

namespace HMP::Gui::Widgets
{

	class Target final: public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		cinolib::DrawablePolygonmesh<> m_mesh;
		const Meshing::Mesher::Mesh& m_sourceMesh;
		bool m_visible;
		std::string m_filename;
		Utils::Transform m_transform;
		cinolib::Color m_faceColor, m_edgeColor;
		bool m_missingMeshFile;
		std::vector<bool> m_edgesPainted;

		void ensureHasMesh() const;

		void updateEdgeColor(Id _eid, const cinolib::Color& _color);

	public:

		Target(const Meshing::Mesher::Mesh& _sourceMesh);

		cpputils::collections::Event<Target> onMeshChanged;
		cpputils::collections::Event<Target, const Mat4&> onApplyTransformToSource;

		const Meshing::Mesher::Mesh& sourceMesh() const;

		bool hasMesh() const;
		const cinolib::DrawablePolygonmesh<>& meshForDisplay() const;

		cinolib::Polygonmesh<> meshForProjection() const;

		void paintEdge(Id _eid, const cinolib::Color& _color);
		void unpaintEdge(Id _eid);

		void show(bool _visible);
		bool visible() const;
		bool& visible();

		cinolib::Color& faceColor();
		const cinolib::Color& faceColor() const;

		cinolib::Color& edgeColor();
		const cinolib::Color& edgeColor() const;

		Utils::Transform& transform();
		const Utils::Transform& transform() const;

		void identity(bool _origin = true, bool _translation = true, bool _rotation = true, bool _scale = true);
		void fit(bool _origin = true, bool _translation = true, bool _scale = true);

		void updateTransform();
		void updateVisibility();
		void updateColor(bool _face = true, bool _edge = true);

		bool load(bool _keepTransform = false);
		void load(const std::string& _filename, bool _keepTransform = false);
		void clearMesh();
		void requestApplyTransformToSource();

		void draw() override;

		void serialize(HMP::Utils::Serialization::Serializer& _serializer) const;
		void deserialize(HMP::Utils::Serialization::Deserializer& _deserializer);

	};

}