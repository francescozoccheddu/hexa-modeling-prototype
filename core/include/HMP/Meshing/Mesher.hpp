#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/drawable_hexmesh.h>
#include <unordered_map>
#include <HMP/Dag/Element.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/range/Ranged.hpp>
#include <HMP/Utils/DerefRanged.hpp>
#include <HMP/Utils/MapRanged.hpp>
#include <cpputils/collections/Event.hpp>
#include <utility>
#include <functional>
#include <unordered_set>
#include <vector>
#include <cinolib/octree.h>
#include <memory>

namespace HMP::Meshing
{

	namespace Internal
	{

		constexpr std::pair<Dag::Element&, Id> mesherEntryConvert(std::unordered_map<Dag::Element*, Id>::value_type& _entry)
		{
			return std::pair<Dag::Element&, Id>{*_entry.first, _entry.second};
		}

		constexpr std::pair<const Dag::Element&, Id> mesherEntryConstConvert(const std::unordered_map<Dag::Element*, Id>::value_type& _entry)
		{
			return std::pair<const Dag::Element&, Id>{*_entry.first, _entry.second};
		}

	}

	class Mesher final: public cpputils::mixins::ReferenceClass, public HMP::Utils::ConstAndNonConstMapRanged<
		std::unordered_map<Dag::Element*, Id>,
		std::pair<const Dag::Element&, Id>,
		Internal::mesherEntryConstConvert,
		std::pair<Dag::Element&, Id>,
		Internal::mesherEntryConvert
	>
	{

	public:

		void add(Dag::Element& _element);

		struct RemovedIds final
		{
			std::vector<Id> vids;
			std::vector<Id> eids;
			std::vector<Id> fids;
			Id pid;
			bool vidsActuallyRemoved;
		};

		class PolyAttributes final: public cinolib::Polyhedron_std_attributes
		{

		private:

			friend void Mesher::add(Dag::Element& _element);

			Dag::Element* m_element;

		public:

			Dag::Element& element();
			const Dag::Element& element() const;

		};

		using Mesh = cinolib::DrawableHexmesh<cinolib::Mesh_std_attributes, cinolib::Vert_std_attributes, cinolib::Edge_std_attributes, cinolib::Polygon_std_attributes, PolyAttributes>;

	private:

		static constexpr Real c_maxVertDistance{ 1e-3 };

		Mesh m_mesh;
		std::unordered_map<Dag::Element*, Id> m_elementToPid;
		bool m_dirty;
		std::vector<Id> m_visibleFaceIndices, m_visibleEdgeIndices;
		RemovedIds m_removedIds;
		std::unique_ptr<cinolib::Octree> m_octree;

		void updateOctree();

	public:

		cinolib::Color edgeColor, faceColor;

		Mesher();

		mutable cpputils::collections::Event<Mesher, const Dag::Element&, const RemovedIds&> onElementRemove;
		mutable cpputils::collections::Event<Mesher, const Dag::Element&, const RemovedIds&> onElementRemoved;
		mutable cpputils::collections::Event<Mesher, const Dag::Element&> onElementAdd;
		mutable cpputils::collections::Event<Mesher, const Dag::Element&> onElementAdded;
		mutable cpputils::collections::Event<Mesher> onClear;
		mutable cpputils::collections::Event<Mesher> onCleared;

		const Mesh& mesh() const;

		bool has(const Dag::Element& _element) const;
		Id elementToPid(const Dag::Element& _element) const;
		Dag::Element& pidToElement(Id _pid);
		const Dag::Element& pidToElement(Id _pid) const;
		//void add(Dag::Element& _element);
		Id addVert(const Vec& _vert);
		void remove(Dag::Element& _element, bool _removeVids);
		void moveVert(Id _vid, const Vec& _position);
		void clear();

		void updateColors(bool _poly = true, bool _edge = true);

		void updateMesh();
		void updateMeshTemp(const std::unordered_set<Id>& _changedVids);

		bool pick(const Vec& _from, const Vec& _dir, Id& _pid, Id& _fid, Id& _eid, Id& _vid, bool _allowBehind = false) const;

	};

}