#pragma once

#include <cpputils/collections/Event.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Projection/project.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Commander.hpp>
#include <cinolib/feature_network.h>
#include <HMP/Utils/Serialization.hpp>
#include <array>
#include <imgui.h>

namespace HMP::Gui::Widgets
{

	class Projection final: public SidebarWidget
	{

	private:

		static constexpr cinolib::KeyBinding c_kbAddPathEdge{ GLFW_KEY_I, GLFW_MOD_SHIFT };
		static constexpr cinolib::KeyBinding c_kbRemovePathEdge{ GLFW_KEY_I, GLFW_MOD_CONTROL };

		using EidsPath = HMP::Projection::Utils::EidsPath;
		using Point = HMP::Projection::Utils::Point;

		HMP::Projection::Options m_options;
		std::vector<EidsPath> m_paths;
		cinolib::FeatureNetworkOptions m_featureFinderOptions;
		bool m_usePathAsCrease;
		float m_featureFinderCreaseAngle;
		bool m_showPaths, m_showAllPaths;
		I m_currentPath;

		void attached();

		void matchPaths(I _first, I _lastEx, bool _fromSource);

		void findPaths(bool _inSource);

		void setSourcePathFromSelection(I _path);

		template<class M, class V, class E, class P>
		void setPathEdgeAtPoint(const Vec& _point, bool _add, const cinolib::AbstractMesh<M, V, E, P>& _mesh, bool _source);

		ImVec4 pathColor(I _path) const;

		void removePath(I _path);

		const HMP::Projection::Options& options() const;

		void requestProjection();

		bool canReproject() const;

		void requestReprojection();

		void setTargetPathEdgeAtPoint(const Vec& _point, bool _add);

		void setSourcePathEdgeAtPoint(const Vec& _point, bool _add);

		void drawSidebar() override;

		void drawCanvas() override;

		void serialize(HMP::Utils::Serialization::Serializer& _serializer) const override;

		void deserialize(HMP::Utils::Serialization::Deserializer& _deserializer) override;

		void printUsage() const override;

		bool keyPressed(const cinolib::KeyBinding& _key) override;

	public:

		float lineThickness{ 1.5f };

		Projection();

	};

}

#define HMP_GUI_WIDGETS_PROJECTION_IMPL
#include <HMP/Gui/Widgets/Projection.tpp>
#undef HMP_GUI_WIDGETS_PROJECTION_IMPL