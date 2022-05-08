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

		using ElementToPidIterable = cpputils::collections::Iterable <
			std::unordered_map<Dag::Element*, Id>,
			std::pair<Dag::Element&, Id>,
			std::pair<const Dag::Element&, Id>,
			[](std::unordered_map<Dag::Element*, Id>::value_type& _entry) { return std::pair<Dag::Element&, Id>{*_entry.first, _entry.second}; },
			[](const std::unordered_map<Dag::Element*, Id>::value_type& _entry) { return std::pair<const Dag::Element&, Id>{*_entry.first, _entry.second}; }
		> ;

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

		using FaceMarkerIterable = cpputils::collections::Iterable <
			std::unordered_set<std::pair<const Dag::Element*, Id>, FaceMarkerHasher>,
			std::pair<const Dag::Element&, Id>,
			std::pair<const Dag::Element&, Id>,
			[](const std::pair<const Dag::Element*, Id>& _entry) { return std::pair<const Dag::Element&, Id>{*_entry.first, _entry.second}; },
			[](const std::pair<const Dag::Element*, Id>& _entry) { return std::pair<const Dag::Element&, Id>{*_entry.first, _entry.second}; }
		> ;

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

		class PolyMarkerSet final : public cpputils::mixins::ReferenceClass, public Internal::PolyMarkerIterable
		{

		private:

			friend class Mesher;

			Mesher& m_mesher;
			std::unordered_set<const Dag::Element*> m_data;

			PolyMarkerSet(Mesher& _mesher);

			void mark(const Dag::Element& _element, bool _marked);

		public:

			bool has(const Dag::Element& _element) const;
			bool add(const Dag::Element& _element);
			bool remove(const Dag::Element& _element);
			bool clear();

		};

		class FaceMarkerSet final : public cpputils::mixins::ReferenceClass, public Internal::FaceMarkerIterable
		{

		private:

			friend class Mesher;

			Mesher& m_mesher;
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

		void updateMesh();

	};

}