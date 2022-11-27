#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/geometry/vec_mat.h>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/meshes/drawable_polygonmesh.h>
#include <cinolib/meshes/polygonmesh.h>
#include <HMP/Meshing/Mesher.hpp>
#include <cinolib/geometry/aabb.h>
#include <cinolib/color.h>
#include <cinolib/drawable_segment_soup.h>
#include <string>
#include <cpputils/collections/Event.hpp>
#include <HMP/Gui/Utils/Transform.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Utils/Serialization.hpp>
#include <utility>
#include <unordered_map>

namespace HMP::Gui::Widgets
{

	class Target final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		cinolib::DrawablePolygonmesh<>* m_mesh;
		const Meshing::Mesher::Mesh& m_sourceMesh;
		bool m_visible;
		std::string m_filename;
		Utils::Transform m_transform;
		cinolib::Color m_faceColor, m_edgeColor;
		bool m_showProjectLines;
		std::vector<std::pair<Vec, Vec>> m_verts;
		cinolib::DrawableSegmentSoup m_projectLines;
		Real m_vertInterpProgress;
		float m_sliderVertInterpProgress;
		bool m_missingMeshFile;

		void ensureHasMesh() const;

	public:

		Target(const Meshing::Mesher::Mesh& _sourceMesh);

		cpputils::collections::Event<Target> onProjectRequest, onMeshLoad, onMeshClear;
		cpputils::collections::Event<Target> onTransform;
		cpputils::collections::Event<Target, const Mat4&> onApplyTransformToSource;
		cpputils::collections::Event<Target, const std::unordered_map<Id, Vec>&> onVertsInterpolationChanged;

		const Meshing::Mesher::Mesh& sourceMesh() const;

		bool hasMesh() const;
		const cinolib::DrawablePolygonmesh<>& mesh() const;

		cinolib::Polygonmesh<> meshForProjection() const;

		const std::vector<std::pair<Vec, Vec>>& verts() const;

		Real vertInterpolationProgress() const;

		void interpolateVerts(Real _progress);

		void cancelVertInterpolation();

		bool interpolatingVerts() const;

		void clearDebugInfo();

		void show(bool _visible);
		bool visible() const;
		bool& visible();

		void showProjectLines(bool _visible);

		void setProjectLinesColor(const cinolib::Color& _color);

		void setProjectLinesThickness(float _thickness);

		void setProjectLinesAlwaysOnFront(bool _alwaysInFront);

		const cinolib::DrawableSegmentSoup& projectLines() const;

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
		void requestProjection();
		void requestApplyTransformToSource();

		void draw() override;

		void serialize(HMP::Utils::Serialization::Serializer& _serializer) const;
		void deserialize(HMP::Utils::Serialization::Deserializer& _deserializer);

	};

}