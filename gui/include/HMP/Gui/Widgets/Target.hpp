#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Meshing/types.hpp>
#include <cinolib/geometry/vec_mat.h>
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

	class Target final: public SidebarWidget
	{

	private:

		cinolib::DrawablePolygonmesh<> m_mesh;
		const Meshing::Mesher::Mesh& m_sourceMesh;
		std::string m_filename;
		bool m_missingMeshFile;

	public:

		cinolib::Color faceColor, edgeColor;
		Utils::Transform transform;
		bool visible;
		float edgeThickness{ 2.0f };

		Target(const Meshing::Mesher::Mesh& _sourceMesh);

		mutable cpputils::collections::Event<Target> onMeshChanged;
		mutable cpputils::collections::Event<Target> onMeshShapeChanged;
		mutable cpputils::collections::Event<Target, const Mat4&> onApplyTransformToSource;

		const Meshing::Mesher::Mesh& sourceMesh() const;

		bool hasMesh() const;
		const cinolib::DrawablePolygonmesh<>& meshForDisplay() const;

		cinolib::Polygonmesh<> meshForProjection() const;

		void identity(bool _origin = true, bool _translation = true, bool _rotation = true, bool _scale = true);
		void fit(bool _origin = true, bool _translation = true, bool _scale = true);

		void updateTransform();
		void updateVisibility();
		void updateColor(bool _face = true, bool _edge = true);
		void updateEdgeThickness();

		bool load(bool _keepTransform = false);
		void load(const std::string& _filename, bool _keepTransform = false);
		void clearMesh();
		void requestApplyTransformToSource();

		void drawSidebar() override;

		void serialize(HMP::Utils::Serialization::Serializer& _serializer) const;
		void deserialize(HMP::Utils::Serialization::Deserializer& _deserializer);

	};

}