#include <HMP/Actions/Extrude.hpp>

#include <HMP/Actions/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/index.hpp>
#include <cpputils/range/enumerate.hpp>
#include <ranges>

namespace HMP::Actions
{

	void Extrude::apply()
	{
		const cpputils::collections::FixedVector<Id, 3>& faceOffsets{ m_operation->faceOffsets() };
		for (const auto& [i, parent] : cpputils::range::enumerate(m_elements))
		{
			for (const Dag::Operation& child : parent->children())
			{
				if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
				{
					throw std::logic_error{ "element has non-extrude child" };
				}
			}
		}
		const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
		const std::vector<Id> pids{ cpputils::range::of(m_elements).map([&](const Dag::Element* _element) {
			return mesher().elementToPid(*_element);
		}).toVector() };
		std::array<Id, 3> fids{ cpputils::range::index<I>(m_elements).map([&](I _i) -> Id {
			return mesh.poly_face_id(pids[_i], faceOffsets[_i]);
		}).toArray<3>(noId) };
		if (m_operation->source() == Dag::Extrude::ESource::Face)
		{
			fids[1] = mesh.poly_face_id(pids[0], faceOffsets[1]);
		}
		switch (m_operation->source())
		{
			case Dag::Extrude::ESource::Vertex:
				if (!mesh.faces_are_adjacent(fids[0], fids[2]) || !mesh.faces_are_adjacent(fids[1], fids[2]))
				{
					throw std::logic_error{ "faces are not adjacent" };
				}
			case Dag::Extrude::ESource::Edge:
			case Dag::Extrude::ESource::Face:
				if (!mesh.faces_are_adjacent(fids[0], fids[1]))
				{
					throw std::logic_error{ "faces are not adjacent" };
				}
				break;
		}
		for (Dag::Element* parent : m_elements)
		{
			m_operation->parents().attach(*parent);
		}
		Utils::applyExtrude(mesher(), *m_operation);
		mesher().updateMesh();
	}

	void Extrude::unapply()
	{
		Utils::unapplyExtrude(mesher(), *m_operation);
		mesher().updateMesh();
	}

	Extrude::Extrude(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Dag::Extrude::ESource _source)
		: m_elements{ _elements }, m_operation{ Utils::prepareExtrude(_faceOffsets, _source) }
	{}

	Extrude::Extrude(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset)
		: Extrude{ {&_element}, {_forwardFaceOffset, _upFaceOffset}, Dag::Extrude::ESource::Face }
	{}

	Extrude::Extrude(Dag::Element& _element0, Id _faceOffset0, Dag::Element& _element1, Id _faceOffset1)
		: Extrude{ {&_element0, &_element1}, {_faceOffset0, _faceOffset1}, Dag::Extrude::ESource::Edge }
	{}

	Extrude::Extrude(Dag::Element& _element0, Id _faceOffset0, Dag::Element& _element1, Id _faceOffset1, Dag::Element& _element2, Id _faceOffset2)
		: Extrude{ {&_element0, &_element1, &_element2}, {_faceOffset0, _faceOffset1, _faceOffset2}, Dag::Extrude::ESource::Vertex }
	{}

	Extrude::Elements Extrude::elements() const
	{
		return cpputils::range::ofc(m_elements).dereference().immutable();
	}

	const Dag::Extrude& Extrude::operation() const
	{
		return *m_operation;
	}

}