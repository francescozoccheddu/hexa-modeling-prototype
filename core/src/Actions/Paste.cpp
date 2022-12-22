#include <HMP/Actions/Paste.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Actions/Utils.hpp>
#include <array>
#include <cpputils/range/of.hpp>

namespace HMP::Actions
{

	std::array<Vec, 3> basis(const Meshing::Mesher& _mesher, const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Id _firstUpFaceOffset)
	{
		throw std::logic_error{ "not implemented yet" };
	}

	void Paste::apply()
	{
		if (!m_prepared)
		{
			m_prepared = true;
			Meshing::Mesher& mesher{ this->mesher() };
			const Meshing::Mesher::Mesh& mesh{ mesher.mesh() };
			Mat4 transform{ Mat4::TRANS(Vec{0,2,0}) };
			Dag::Utils::transform(*m_operation, transform);
		}
		for (Dag::Element* parent : m_elements)
		{
			m_operation->parents().attach(*parent);
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

	Paste::Paste(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Id _firstUpFaceOffset, const Dag::Extrude& _source)
		: m_elements{ _elements }, m_operation{ static_cast<Dag::Extrude&>(Dag::Utils::clone(_source)) }, m_prepared{ false }
	{}

	Paste::Elements Paste::elements() const
	{
		return cpputils::range::of(m_elements).dereference().immutable();
	}

	const Dag::Extrude& Paste::operation() const
	{
		return *m_operation;
	}

}