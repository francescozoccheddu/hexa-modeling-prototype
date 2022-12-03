#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Gui/Utils/Transform.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <cpputils/collections/Iterable.hpp>
#include <unordered_map>
#include <vector>

namespace HMP::Gui::Widgets
{

	class VertEdit final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		static constexpr Id vertsIterableConvert(const std::unordered_map<Id, Vec>::value_type& _entry)
		{
			return _entry.first;
		}

		Meshing::Mesher& m_mesher;
		Utils::Transform m_appliedTransform, m_unappliedTransform;
		std::unordered_map<Id, Vec> m_verts;
		Vec m_centroid;
		bool m_pendingAction;

		void addOrRemove(const Id* _vids, I _count, bool _add);

	public:

		using Vids = cpputils::collections::Iterable<const std::unordered_map<Id, Vec>, Id, Id, vertsIterableConvert, vertsIterableConvert>;

		cpputils::collections::Event<VertEdit> onVidsChanged{};
		cpputils::collections::Event<VertEdit> onCentroidChanged{};
		cpputils::collections::Event<VertEdit> onMeshUpdated{};
		cpputils::collections::Event<VertEdit> onPendingActionChanged{};
		cpputils::collections::Event<VertEdit, std::vector<Id>, Mat4> onApplyAction{};

		VertEdit(Meshing::Mesher& _mesher);

		void add(Id _vid);

		void remove(Id _vid);

		void add(const std::vector<Id>& _vids);

		void remove(const std::vector<Id>& _vids);

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

		void draw() override;

	};

}