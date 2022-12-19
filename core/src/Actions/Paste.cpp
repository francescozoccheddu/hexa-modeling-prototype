#include <HMP/Actions/Paste.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Actions/Utils.hpp>

namespace HMP::Actions
{

	void Paste::apply()
	{
		m_operation->parents().attach(m_element);
		if (!m_prepared)
		{
			m_prepared = true;
			Utils::applyTree(mesher(), *m_operation);
		}
		Meshing::Utils::addLeafs(mesher(), *m_operation, false);
		mesher().updateMesh();
	}

	void Paste::unapply()
	{
		Meshing::Utils::removeLeafs(mesher(), *m_operation);
		m_operation->parents().detachAll(false);
		mesher().updateMesh();
	}

	Paste::Paste(Dag::Element& _target, Id _targetForwardFaceOffset, Id _targetUpFaceOffset, Dag::Extrude& _source)
		: m_element{ _target }, m_operation{ static_cast<Dag::Extrude&>(Dag::Utils::clone(_source)) }, m_prepared{ false }
	{
		// TODO!
		//m_operation->forwardFaceOffset() = _targetForwardFaceOffset;
		//m_operation->upFaceOffset() = _targetUpFaceOffset;
	}

	const Dag::Element& Paste::element() const
	{
		return m_element;
	}

	const Dag::Extrude& Paste::operation() const
	{
		return *m_operation;
	}

}