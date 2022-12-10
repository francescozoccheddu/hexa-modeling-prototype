#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <cpputils/collections/Event.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Meshing/Projection.hpp>
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

		Meshing::Projection::Options m_options;
		Widgets::Target& m_targetWidget;
		HMP::Commander& m_commander;
		HMP::Meshing::Mesher& m_mesher;
		std::vector<Meshing::Projection::Path> m_creases;
		VertEdit& m_vertEditWidget;
		cinolib::FeatureNetworkOptions m_featureFinderOptions;
		bool m_showCreases{ true }, m_showAllCreases{ false };
		I m_currentCrease{};

		void matchCreases(I _first, I _lastEx, bool _fromSource);

		void findCreases(bool _inSource);

		void setSourceCreaseFromSelection(I _crease);

		void updateMeshEdges(I _first, I _lastEx, bool _show);

		void updateSourceMeshEdges(I _first, I _lastEx, bool _show);

		void updateTargetMeshEdges(I _first, I _lastEx, bool _show);

		void clearCreases();

		void addCrease();

		void removeCrease(I _index);

		void clearSourceCreases(I _first, I _lastEx);

		void clearTargetCreases(I _first, I _lastEx);

		std::array<Id, 2> getEdgeVids(Id _eid, bool _source);
		std::array<Id, 3> getEdgeVids(Id _eid0, Id _eid1, bool _source);

		template<class M, class V, class E, class P>
		void setCreaseEdgeAtPoint(const Vec& _point, bool _add, const cinolib::AbstractMesh<M, V, E, P>& _mesh, bool _source);

	public:

		Projection(Widgets::Target& _targetWidget, HMP::Commander& _commander, HMP::Meshing::Mesher& _mesher, VertEdit& _vertEditWidget);

		cpputils::collections::Event<Projection, const std::vector<Meshing::Projection::Point>&, const std::vector<Meshing::Projection::Path>&, const Meshing::Projection::Options&> onProjectRequest;

		const Meshing::Projection::Options& options() const;

		void requestProjection();

		bool canReproject() const;

		void requestReprojection();

		void setTargetCreaseEdgeAtPoint(const Vec& _point, bool _add);

		void setSourceCreaseEdgeAtPoint(const Vec& _point, bool _add);

		void draw() override;

	};

}

#define HMP_GUI_WIDGETS_PROJECTION_IMPL
#include <HMP/Gui/Widgets/Projection.tpp>
#undef HMP_GUI_WIDGETS_PROJECTION_IMPL