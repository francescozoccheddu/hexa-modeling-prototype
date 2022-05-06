#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/drawable_hexmesh.h>
#include <unordered_map>
#include <HMP/Dag/Element.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/Iterable.hpp>
#include <cpputils/collections/DereferenceIterable.hpp>
#include <utility>

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

	private:

		Mesh m_mesh;
		std::unordered_map<Dag::Element*, Id> m_elementToPid;

		Id getOrAddVert(const Vec& _vert);

	public:

		Mesher();

		const Mesh& mesh() const;

		bool getVert(const Vec& _vert, Id& _vid) const;
		Id getVert(const Vec& _vert) const;
		bool hasVert(const Vec& _vert) const;

		bool has(const Dag::Element& _element) const;
		Id elementToPid(const Dag::Element& _element) const;
		Dag::Element& pidToElement(Id _pid);
		const Dag::Element& pidToElement(Id _pid) const;
		//void add(Dag::Element& _element);
		void remove(Dag::Element& _element);
		void moveVert(Id _vid, const Vec& _position);
		void clear();

	};

}