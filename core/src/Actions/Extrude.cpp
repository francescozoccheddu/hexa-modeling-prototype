#include <HMP/Actions/Extrude.hpp>

#include <HMP/Actions/Utils.hpp>
#include <cpputils/collections/conversions.hpp>
#include <cpputils/collections/zip.hpp>
#include <cpputils/collections/index.hpp>
#include <ranges>

namespace HMP::Actions
{

	void Extrude::apply()
	{
		for (const auto& [parent, i] : cpputils::collections::zip(m_elements, cpputils::collections::index(m_elements)))
		{
			for (const Dag::Operation& child : parent->children())
			{
				if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
				{
					throw std::logic_error{ "element has non-extrude child" };
				}
				if (static_cast<const Dag::Extrude&>(child).faceOffsets()[i] == m_operation->faceOffsets()[i])
				{
					throw std::logic_error{ "element already has equivalent child" };
				}
			}
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

	Extrude::Extrude(const std::vector<Dag::Element*>& _elements, const std::array<Id, 3>& _faceOffsets, Dag::Extrude::ESource _source)
		: m_elements{ _elements }, m_operation{ Utils::prepareExtrude(_faceOffsets, _source) }
	{}

	Extrude::Extrude(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset)
		: Extrude{ {&_element}, {_forwardFaceOffset, _upFaceOffset, noId}, Dag::Extrude::ESource::Face }
	{}

	Extrude::Extrude(Dag::Element& _element0, Id _faceOffset0, Dag::Element& _element1, Id _faceOffset1)
		: Extrude{ {&_element0, &_element1}, {_faceOffset0, _faceOffset1, noId}, Dag::Extrude::ESource::Edge }
	{}

	Extrude::Extrude(Dag::Element& _element0, Id _faceOffset0, Dag::Element& _element1, Id _faceOffset1, Dag::Element& _element2, Id _faceOffset2)
		: Extrude{ {&_element0, &_element1, &_element2}, {_faceOffset0, _faceOffset1, _faceOffset2}, Dag::Extrude::ESource::Vertex }
	{}

	Extrude::Elements Extrude::elements() const
	{
		return Elements{ m_elements };
	}

	const Dag::Extrude& Extrude::operation() const
	{
		return *m_operation;
	}

}