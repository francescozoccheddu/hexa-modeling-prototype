#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <unordered_set>

namespace HMP::Gui::Widgets
{

	class VertEdit final : public cinolib::SideBarItem, public cpputils::mixins::ReferenceClass
	{

	private:

		static constexpr Id vertsIterableConvert(std::pair<const Id, Vec>& _entry)
		{
			return _entry.first;
		}

		static constexpr Id vertsIterableConvertConst(const std::pair<const Id, Vec>& _entry)
		{
			return _entry.first;
		}

		Meshing::Mesher& m_mesher;
		Vec m_centroid;
		std::unordered_set<Id> m_vids;
		Mat4 m_transform;
		bool m_pendingAction;

		cinolib::vec3<float> m_translationSlider, m_scaleSlider, m_rotationSlider;

		bool apply(const Mat4& _transform, bool _updateMesh = true);
		bool applyAndCombine(const Mat4& _transform);
		bool isIdentity() const;

		void addOrRemove(const Id* _vids, std::size_t _count, bool _add);

		void cancel(bool _updateMesh);

	public:

		cpputils::collections::Event<VertEdit> onVidsChanged;
		cpputils::collections::Event<VertEdit> onCentroidChanged;
		cpputils::collections::Event<VertEdit> onMeshUpdated;
		cpputils::collections::Event<VertEdit> onPendingActionChanged;
		cpputils::collections::Event<VertEdit, std::unordered_set<Id>, Mat4> onApplyAction;

		VertEdit(Meshing::Mesher& _mesher);

		void add(Id _vid);

		void remove(Id _vid);

		void add(const std::vector<Id>& _vids);

		void remove(const std::vector<Id>& _vids);

		bool has(Id _vid) const;

		const std::unordered_set<Id>& vids() const;

		void clear();

		bool empty() const;

		const Vec& centroid() const;

		bool pendingAction() const;

		void cancel();

		void applyAction();

		bool translate(const Vec& _offset);

		bool rotate(const Vec& _normAxis, Real _angleDeg);

		bool scale(const Vec& _amount);

		void update();

		void draw() override;

	};

}