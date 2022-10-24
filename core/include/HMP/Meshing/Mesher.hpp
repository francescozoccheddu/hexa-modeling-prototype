#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/drawable_hexmesh.h>
#include <unordered_map>
#include <HMP/Dag/Element.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Iterable.hpp>
#include <cpputils/collections/DereferenceIterable.hpp>
#include <utility>
#include <functional>

namespace HMP::Meshing
{

	namespace Internal
	{

		inline constexpr std::pair<Dag::Element&, Id> ElementToPidIterableConvert(std::unordered_map<Dag::Element*, Id>::value_type& _entry)
		{
			return std::pair<Dag::Element&, Id>{*_entry.first, _entry.second};
		}

		inline constexpr std::pair<const Dag::Element&, Id> ElementToPidIterableConstConvert(const std::unordered_map<Dag::Element*, Id>::value_type& _entry)
		{
			return std::pair<const Dag::Element&, Id>{*_entry.first, _entry.second};
		}

		using ElementToPidIterable = cpputils::collections::Iterable <
			std::unordered_map<Dag::Element*, Id>,
			std::pair<Dag::Element&, Id>,
			std::pair<const Dag::Element&, Id>,
			ElementToPidIterableConvert,
			ElementToPidIterableConstConvert
		>;

		using PolyMarkerIterable = cpputils::collections::DereferenceIterable <
			std::unordered_set<const Dag::Element*>,
			const Dag::Element&,
			const Dag::Element&
		>;

		struct FaceMarkerHasher final
		{
			std::size_t operator () (const std::pair<const Dag::Element*, Id>& _faceMarker) const
			{
				return std::hash<const Dag::Element*>{}(_faceMarker.first);
			}
		};

		inline constexpr std::pair<const Dag::Element&, Id> FaceMarkerIterableConstConvert(const std::pair<const Dag::Element*, Id>& _entry)
		{
			return std::pair<const Dag::Element&, Id>{*_entry.first, _entry.second};
		}

		using FaceMarkerIterable = cpputils::collections::Iterable <
			std::unordered_set<std::pair<const Dag::Element*, Id>, FaceMarkerHasher>,
			std::pair<const Dag::Element&, Id>,
			std::pair<const Dag::Element&, Id>,
			FaceMarkerIterableConstConvert,
			FaceMarkerIterableConstConvert
		>;

	}

	class Mesher final : public cpputils::mixins::ReferenceClass, public Internal::ElementToPidIterable
	{

	public:

		void add(Dag::Element& _element);

		class PolyAttributes final : public cinolib::Polyhedron_std_attributes
		{

		private:

			friend void Mesher::add(Dag::Element& _element);

			Dag::Element* m_element;

		public:

			Dag::Element& element();
			const Dag::Element& element() const;

		};

		using Mesh = cinolib::DrawableHexmesh<cinolib::Mesh_std_attributes, cinolib::Vert_std_attributes, cinolib::Edge_std_attributes, cinolib::Polygon_std_attributes, PolyAttributes>;

		class PolyMarkerSet;
		class FaceMarkerSet;

		class MarkerSetBase : public cpputils::mixins::ReferenceClass
		{

		private:

			friend class PolyMarkerSet;
			friend class FaceMarkerSet;

			Mesher& m_mesher;
			bool m_dirty;
			cinolib::Color m_color;

			MarkerSetBase(Mesher& _mesher);

		public:

			cinolib::Color& color();
			const cinolib::Color& color() const;

			void requestUpdate();

		};

		class PolyMarkerSet final : public MarkerSetBase, public Internal::PolyMarkerIterable
		{

		private:

			friend class Mesher;

			using MarkerSetBase::m_dirty;

			std::unordered_set<const Dag::Element*> m_data;

			PolyMarkerSet(Mesher& _mesher);

			void mark(const Dag::Element& _element, bool _marked);

		public:

			bool has(const Dag::Element& _element) const;
			bool add(const Dag::Element& _element);
			bool remove(const Dag::Element& _element);
			bool clear();

		};

		class FaceMarkerSet final : public MarkerSetBase, public Internal::FaceMarkerIterable
		{

		private:

			friend class Mesher;

			using MarkerSetBase::m_dirty;

			std::unordered_set < std::pair<const Dag::Element*, Id>, Internal::FaceMarkerHasher> m_data;

			FaceMarkerSet(Mesher& _mesher);

			void mark(const Dag::Element& _element, Id _faceOffset, bool _marked);

		public:

			bool has(const Dag::Element& _element, Id _faceOffset) const;
			bool add(const Dag::Element& _element, Id _faceOffset);
			bool addAll(const Dag::Element& _element);
			bool remove(const Dag::Element& _element, Id _faceOffset);
			bool removeAll(const Dag::Element& _element);
			bool clear();

		};

	private:

		Mesh m_mesh;
		std::unordered_map<Dag::Element*, Id> m_elementToPid;
		FaceMarkerSet m_faceMarkerSet;
		PolyMarkerSet m_polyMarkerSet;
		bool m_dirty;
		cinolib::Color m_polyColor, m_edgeColor;

		Id getOrAddVert(const Vec& _vert);

	public:

		Mesher();

		const Mesh& mesh() const;

		Id getVert(const Vec& _vert) const;

		bool has(const Dag::Element& _element) const;
		Id elementToPid(const Dag::Element& _element) const;
		Dag::Element& pidToElement(Id _pid);
		const Dag::Element& pidToElement(Id _pid) const;
		//void add(Dag::Element& _element);
		void remove(Dag::Element& _element);
		void moveVert(Id _vid, const Vec& _position);
		void clear();

		PolyMarkerSet& polyMarkerSet();
		const PolyMarkerSet& polyMarkerSet() const;
		FaceMarkerSet& faceMarkerSet();
		const FaceMarkerSet& faceMarkerSet() const;

		cinolib::Color& polyColor();
		const cinolib::Color& polyColor() const;

		cinolib::Color& edgeColor();
		const cinolib::Color& edgeColor() const;

		void updateColors(bool _poly = true, bool _edge = true);

		void updateMesh();
		void updateMeshMarkers();

		bool pick(const Vec& _from, const Vec& _dir, Id& _pid, Id& _fid, Id& _eid, Id& _vid) const;

	};

}