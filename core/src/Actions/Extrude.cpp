#include <HMP/Actions/Extrude.hpp>

#include <HMP/Actions/ExtrudeUtils.hpp>
#include <cpputils/range/zip.hpp>
#include <cpputils/range/index.hpp>
#include <cpputils/range/enumerate.hpp>

namespace HMP::Actions
{

	void Extrude::apply()
	{
		for (Dag::Element* parent : m_elements)
		{
			m_operation->parents().attach(*parent);
		}
		Dag::Element& child{ m_operation->children().single() };
		m_operation->children().single().vids = ExtrudeUtils::apply(mesher(), *m_operation);
		mesher().add_TOPM(child);
		mesher().updateMesh();
	}

	void Extrude::unapply()
	{
		m_operation->parents().detachAll(false);
		mesher().remove(m_operation->children().single(), true);
		mesher().updateMesh();
	}

	Extrude::Extrude(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<I, 3>& _fis, I _firstVi, bool _clockwise)
		: m_elements{ _elements }, m_operation{ ExtrudeUtils::prepare(_fis, _firstVi, _clockwise) }
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