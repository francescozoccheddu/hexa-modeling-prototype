#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <cinolib/gl/canvas_gui_item.h>
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

	class VertEdit final: public cinolib::SideBarItem, public cinolib::CanvasGuiItem, public cpputils::mixins::ReferenceClass
	{

	private:

		static constexpr Id vertsToVidsConvert(const std::unordered_map<Id, Vec>::value_type& _entry)
		{
			return _entry.first;
		}

		Meshing::Mesher& m_mesher;
		Utils::Transform m_appliedTransform, m_unappliedTransform;
		std::unordered_map<Id, Vec> m_verts;
		Vec m_centroid;
		bool m_pendingAction;

		bool addOrRemove(const Id* _vids, I _count, bool _add, bool _update);

	public:

		cinolib::Color color{ cinolib::Color::YELLOW() };

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

		bool replace(Id _oldVid, Id _newVid);

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

		void draw(const cinolib::GLcanvas& _canvas) override;

	};

}