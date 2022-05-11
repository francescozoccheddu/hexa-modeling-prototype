#include <HMP/Actions/Extrude.hpp>

#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	void Extrude::apply()
	{
		for (const Dag::Operation& child : m_element.children())
		{
			if (child.primitive() != Dag::Operation::EPrimitive::Extrude)
			{
				throw std::logic_error{ "element has non-extrude child" };
			}
			if (static_cast<const Dag::Extrude&>(child).forwardFaceOffset() == m_operation->forwardFaceOffset())
			{
				throw std::logic_error{ "element already has equivalent child" };
			}
		}
		m_operation->parents().attach(m_element);
		Utils::applyExtrude(mesher(), *m_operation);
		mesher().updateMesh();
	}

	void Extrude::unapply()
	{
		Utils::unapplyExtrude(mesher(), *m_operation);
		mesher().updateMesh();
	}

	Extrude::Extrude(Dag::Element& _element, Id _forwardFaceOffset, Id _upFaceOffset)
		: m_element{ _element }, m_operation{ Utils::prepareExtrude(_forwardFaceOffset, _upFaceOffset) }
	{}

	const Dag::Element& Extrude::element() const
	{
		return m_element;
	}

	const Dag::Extrude& Extrude::operation() const
	{
		return *m_operation;
	}

}