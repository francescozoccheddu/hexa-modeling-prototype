#pragma once

#include <HMP/Gui/SidebarWidget.hpp>
#include <HMP/Meshing/types.hpp>
#include <cinolib/color.h>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Gui/Utils/Transform.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <cpputils/range/of.hpp>
#include <unordered_map>
#include <vector>

namespace HMP::Gui::Widgets
{

	class VertEdit final: public SidebarWidget
	{

	private:

		static constexpr Id vertsToVidsConvert(const std::unordered_map<Id, Vec>::value_type& _entry)
		{
			return _entry.first;
		}

		Meshing::Mesher& m_mesher;
		std::unordered_map<Id, Vec> m_verts;
		bool m_pendingAction;
		Utils::Transform m_unappliedTransform, m_appliedTransform;
		Vec m_centroid;

		bool addOrRemove(const Id* _vids, I _count, bool _add, bool _update);

	public:

		float radius{ 6.0f }, lineThickness{ 1.5f };

		using Vids = decltype(cpputils::range::ofc(m_verts).map(&vertsToVidsConvert));

		cpputils::collections::Event<VertEdit> onVidsChanged{};
		cpputils::collections::Event<VertEdit> onCentroidChanged{};
		cpputils::collections::Event<VertEdit> onMeshUpdated{};
		cpputils::collections::Event<VertEdit> onPendingActionChanged{};
		cpputils::collections::Event<VertEdit, std::vector<Id>, Mat4> onApplyAction{};

		VertEdit(Meshing::Mesher& _mesher);

		bool add(Id _vid, bool _update = true);

		bool remove(Id _vid, bool _update = true);

		bool add(const std::vector<Id>& _vids, bool _update = true);

		bool remove(const std::vector<Id>& _vids, bool _update = true);

		bool has(Id _vid) const;

		Vids vids() const;

		void clear();

		bool empty() const;

		const Vec& centroid() const;

		Utils::Transform& transform();

		const Utils::Transform& transform() const;

		void applyTransform();

		bool pendingAction() const;

		void cancel();

		void applyAction();

		void updateCentroid();

		void drawSidebar() override;

		void draw(const cinolib::GLcanvas& _canvas) override;

	};

}