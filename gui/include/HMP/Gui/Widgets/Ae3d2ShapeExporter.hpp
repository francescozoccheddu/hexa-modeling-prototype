#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Meshing/types.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <vector>
#include <cinolib/gl/FreeCamera.hpp>
#include <cinolib/color.h>
#include <string>
#include <optional>

namespace HMP::Gui::Widgets
{

	class Ae3d2ShapeExporter final : public SidebarWidget
	{

	private:

		struct Keyframe final
		{
			std::vector<std::vector<Vec>> polygons{};
			cinolib::FreeCamera<Real> camera{};
		};

		const Meshing::Mesher::Mesh& m_mesh;
		const cinolib::FreeCamera<Real>& m_camera;
		const Target& m_targetWidget;
		std::vector<Keyframe> m_keyframes;
		std::optional<std::string> m_sampleError;

		static bool exportKeyframes(const std::vector<Ae3d2ShapeExporter::Keyframe>& _keyframes);

		bool requestExport() const;

		bool requestTargetExport() const;

		bool requestSample();

		void clear();

		I keyframeCount() const;

		bool empty() const;

		void drawSidebar() override;

	public:

		Ae3d2ShapeExporter(const Meshing::Mesher::Mesh& _mesh, const cinolib::FreeCamera<Real>& _camera, const Target& _targetWidget);

	};

}