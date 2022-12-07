#pragma once

#include <cinolib/gl/side_bar_item.h>
#include <cpputils/collections/Event.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <HMP/Algorithms/Projection.hpp>
#include <HMP/Gui/Widgets/Target.hpp>
#include <HMP/Gui/Widgets/VertEdit.hpp>
#include <HMP/Commander.hpp>
#include <cinolib/feature_network.h>

namespace HMP::Gui::Widgets
{

	class Projection final: public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		using EdgeChain = std::vector<Id>;

		struct EdgeChainPair final
		{
			EdgeChain source, target;
		};

		Algorithms::Projection::Options m_options;
		Widgets::Target& m_targetWidget;
		HMP::Commander& m_commander;
		HMP::Meshing::Mesher& m_mesher;
		std::vector<EdgeChainPair> m_creases;
		VertEdit& m_vertEditWidget;
		cinolib::FeatureNetworkOptions m_featureFinderOptions;
		bool m_showCreases{ false }, m_showAllCreases{ false };
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

		template<class M, class V, class E, class P>
		void setCreaseEdgeAtPoint(const Vec& _point, bool _add, const cinolib::AbstractMesh<M, V, E, P>& _mesh, bool _source);

	public:

		Projection(Widgets::Target& _targetWidget, HMP::Commander& _commander, HMP::Meshing::Mesher& _mesher, VertEdit& _vertEditWidget);

		cpputils::collections::Event<Projection, const Algorithms::Projection::Options&> onProjectRequest;

		const Algorithms::Projection::Options& options() const;

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