#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/gl/side_bar_item.h>
#include <HMP/Meshing/Mesher.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Event.hpp>
#include <cpputils/collections/Iterable.hpp>
#include <unordered_map>

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
		std::unordered_map<Id, Vec> m_verts;

	public:

		using Verts = cpputils::collections::Iterable<std::unordered_map<Id, Vec>, Id, Id, vertsIterableConvert, vertsIterableConvertConst>;

		cpputils::collections::Event<VertEdit, const VertEdit&> onSelectionUpdate;
		cpputils::collections::Event<VertEdit, const VertEdit&> onMeshUpdate;
		cpputils::collections::Event<VertEdit, const VertEdit&> onConsolidate;

		VertEdit(Meshing::Mesher& _mesher);

		void add(Id _vid);

		void remove(Id _vid);

		bool has(Id _vid) const;

		Verts verts() const;

		void clear();

		bool empty() const;

		const Vec& centroid() const;

		bool planar() const;

		void consolidate();

		const Vec& extent() const;

		const Vec& planarExtent() const;

		const Vec& planarRotation() const;

		void translate(const Vec& _offset, bool _consolidate = true);

		void rotate(const Vec& _axis, double _angleDeg, bool _consolidate = true);

		void scale(double _amount, bool _consolidate = true);

		void draw() override;

	};

}