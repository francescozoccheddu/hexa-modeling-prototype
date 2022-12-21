#include <HMP/Actions/Extrude.hpp>

#include <HMP/Actions/Utils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/index.hpp>
#include <cpputils/range/enumerate.hpp>

namespace HMP::Actions
{

	void Extrude::apply()
	{
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

	Extrude::Extrude(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Id _firstUpFaceOffset)
		: m_elements{ _elements }, m_operation{ Utils::prepareExtrude(_firstUpFaceOffset, _faceOffsets) }
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