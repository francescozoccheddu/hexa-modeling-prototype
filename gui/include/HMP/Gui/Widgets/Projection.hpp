#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <cinolib/gl/canvas_gui_item.h>
#include <cpputils/collections/Event.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Projection/project.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Commander.hpp>
#include <cinolib/feature_network.h>
#include <array>
#include <imgui.h>

namespace HMP::Gui::Widgets
{

	class Projection final: public cinolib::SideBarItem, public cinolib::CanvasGuiItem, public cpputils::mixins::ReferenceClass
	{

	private:

		const Widgets::Target& m_targetWidget;
		HMP::Commander& m_commander;
		const HMP::Meshing::Mesher& m_mesher;
		HMP::Projection::Options m_options;
		std::vector<HMP::Projection::Utils::EidsPath> m_paths;
		cinolib::FeatureNetworkOptions m_featureFinderOptions;
		bool m_showPaths, m_showAllPaths;
		I m_currentPath;

		void matchPaths(I _first, I _lastEx, bool _fromSource);

		void findPaths(bool _inSource);

		void setSourcePathFromSelection(I _path);

		template<class M, class V, class E, class P>
		void setPathEdgeAtPoint(const Vec& _point, bool _add, const cinolib::AbstractMesh<M, V, E, P>& _mesh, bool _source);

		ImVec4 pathColor(I _path) const;

	public:

		Projection(const Widgets::Target& _targetWidget, HMP::Commander& _commander, const HMP::Meshing::Mesher& _mesher);

		cpputils::collections::Event<Projection, const cinolib::Polygonmesh<>&, const std::vector<HMP::Projection::Utils::Point>&, const std::vector<HMP::Projection::Utils::EidsPath>&, const HMP::Projection::Options&> onProjectRequest;

		const HMP::Projection::Options& options() const;

		void requestProjection();

		bool canReproject() const;

		void requestReprojection();

		void setTargetPathEdgeAtPoint(const Vec& _point, bool _add);

		void setSourcePathEdgeAtPoint(const Vec& _point, bool _add);

		void draw() override;

		void draw(const cinolib::GLcanvas& _canvas) override;

	};

}

#define HMP_GUI_WIDGETS_PROJECTION_IMPL
#include <HMP/Gui/Widgets/Projection.tpp>
#undef HMP_GUI_WIDGETS_PROJECTION_IMPL