#include <HMP/Actions/Paste.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Actions/Utils.hpp>
#include <array>
#include <algorithm>
#include <cpputils/range/of.hpp>

namespace HMP::Actions
{

	std::array<Vec, 3> basis(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude)
	{
		const PolyVerts verts{ Utils::shapeExtrude(_mesher, _extrude) };
		std::array<std::size_t, 3> indices{ 1,4,3 };
		if (_extrude.clockwise())
		{
			std::reverse(indices.begin(), indices.end());
		}
		return cpputils::range::of(indices).map([&](std::size_t _i) { return verts[_i] - verts[0]; }).toArray();
	}

	void Paste::apply()
	{
		for (Dag::Element* parent : m_elements)
		{
			m_operation->parents().attach(*parent);
		}
		if (!m_prepared)
		{
			m_prepared = true;
			const std::array<Vec, 3> oldBasis{ basis(mesher(), m_sourceOperation) };
			const std::array<Vec, 3> newBasis{ basis(mesher(), *m_operation) };
			Mat4 transform{ Mat4::TRANS(Vec{0,2,0}) };
			Dag::Utils::transform(*m_operation, transform);
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

	Paste::Paste(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<Id, 3>& _faceOffsets, Id _vertOffset, bool _clockwise, const Dag::Extrude& _source)
		: m_elements{ _elements }, m_operation{ static_cast<Dag::Extrude&>(Dag::Utils::clone(_source)) }, m_prepared{ false }, m_sourceOperation{ _source }
	{
		m_operation->faceOffsets() = _faceOffsets;
		m_operation->clockwise() = _clockwise;
		m_operation->vertOffset() = _vertOffset;
	}

	Paste::Elements Paste::elements() const
	{
		return cpputils::range::of(m_elements).dereference().immutable();
	}

	const Dag::Extrude& Paste::operation() const
	{
		return *m_operation;
	}

}