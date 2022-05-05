#pragma once

#include <HMP/Meshing/types.hpp>
#include <cinolib/meshes/drawable_hexmesh.h>
#include <unordered_map>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Operation.hpp>
#include <HMP/Utils/Event.hpp>
#include <HMP/Utils/Mixins.hpp>

namespace HMP
{
	class Project;
}

namespace HMP::Meshing
{

	class Mesher final : public Utils::Mixins::ReferenceClass
	{

	public:

		class PolyAttributes final : cinolib::Polyhedron_std_attributes
		{

		private:

			friend class Mesher;

			Dag::Element* m_element;

		public:

			const Dag::Element& element() const;

		};

		class ProjectClient final
		{

		private:

			friend class HMP::Project;

			ProjectClient() = delete;

		public:

			static void apply(Mesher& _mesher, Dag::Operation& _operation);
			static void unapply(Mesher& _mesher, Dag::Operation& _operation);
			static void clear(Mesher& _mesher);

		};

		using Mesh = cinolib::DrawableHexmesh<cinolib::Mesh_std_attributes, cinolib::Vert_std_attributes, cinolib::Edge_std_attributes, cinolib::Polygon_std_attributes, PolyAttributes>;

	private:

		Mesh m_mesh;
		std::unordered_map<const Dag::Element*, Id> m_elementToPid;

		bool getVert(const Vec& _vert, Id& _vid) const;
		Id getVert(const Vec& _vert) const;
		Id getOrAddVert(const Vec& _vert);
		bool hasVert(const Vec& _vert) const;

		void addPoly(Dag::Element& _element);
		void removePoly(Dag::Element& _element);
		void apply(Dag::Operation& _operation);
		void unapply(Dag::Operation& _operation);
		void clear();

	public:

		struct
		{
			Utils::Event<Mesher, Id> polyAdding{};
			Utils::Event<Mesher, Id> polyAdded{};
			Utils::Event<Mesher, Id> polyRemoving{};
			Utils::Event<Mesher, Id> polyRemoved{};
			Utils::Event<Mesher> clearing{};
			Utils::Event<Mesher> cleared{};
			Utils::Event<Mesher, Id, Id> polyIdChanging{};
			Utils::Event<Mesher, Id, Id> polyIdChanged{};
		} events{};

		const Mesh& mesh() const;

		bool& pidMarked(Id _pid);
		bool pidMarked(Id _pid) const;
		bool& fidMarked(Id _fid);
		bool fidMarked(Id _fid) const;

		bool isElementActive(const Dag::Element& _element) const;
		Id elementToPid(const Dag::Element& _element) const;

	};

}