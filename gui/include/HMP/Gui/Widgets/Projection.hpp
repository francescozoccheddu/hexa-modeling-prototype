#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <cpputils/collections/Event.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Projection/project.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Commander.hpp>
#include <cinolib/feature_network.h>
#include <array>

namespace HMP::Gui::Widgets
{

	class Projection final: public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		HMP::Projection::Options m_options;
		Widgets::Target& m_targetWidget;
		HMP::Commander& m_commander;
		HMP::Meshing::Mesher& m_mesher;
		std::vector<HMP::Projection::Utils::EidsPath> m_paths;
		VertEdit& m_vertEditWidget;
		cinolib::FeatureNetworkOptions m_featureFinderOptions;
		bool m_showPaths{ true }, m_showAllPaths{ false };
		I m_currentPath{};

		void matchPaths(I _first, I _lastEx, bool _fromSource);

		void findPaths(bool _inSource);

		void setSourcePathFromSelection(I _path);

		void updateMeshEdges(I _first, I _lastEx, bool _show, bool _source);

		void updateBothMeshEdges(I _first, I _lastEx, bool _show);

		void updateSourceMeshEdges(I _first, I _lastEx, bool _show);

		void updateTargetMeshEdges(I _first, I _lastEx, bool _show);

		void clearBothPaths();

		void addPath();

		void removePath(I _index);

		void clearPaths(I _first, I _lastEx, bool _source);

		void clearBothPaths(I _first, I _lastEx);

		void clearSourcePaths(I _first, I _lastEx);

		void clearTargetPaths(I _first, I _lastEx);

		template<class M, class V, class E, class P>
		void setPathEdgeAtPoint(const Vec& _point, bool _add, const cinolib::AbstractMesh<M, V, E, P>& _mesh, bool _source);

	public:

		Projection(Widgets::Target& _targetWidget, HMP::Commander& _commander, HMP::Meshing::Mesher& _mesher, VertEdit& _vertEditWidget);

		cpputils::collections::Event<Projection, const std::vector<HMP::Projection::Utils::Point>&, const std::vector<HMP::Projection::Utils::EidsPath>&, const HMP::Projection::Options&> onProjectRequest;

		const HMP::Projection::Options& options() const;

		void requestProjection();

		bool canReproject() const;

		void requestReprojection();

		void setTargetPathEdgeAtPoint(const Vec& _point, bool _add);

		void setSourcePathEdgeAtPoint(const Vec& _point, bool _add);

		void draw() override;

	};

}

#define HMP_GUI_WIDGETS_PROJECTION_IMPL
#include <HMP/Gui/Widgets/Projection.tpp>
#undef HMP_GUI_WIDGETS_PROJECTION_IMPL