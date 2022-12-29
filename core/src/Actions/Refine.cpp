#include <HMP/Actions/Refine.hpp>

#include <HMP/Refinement/Utils.hpp>
#include <cpputils/range/of.hpp>
#include <cassert>
#include <queue>

namespace HMP::Actions
{

	std::vector<std::pair<Dag::Element*, Dag::Refine*>> prepareRecursive(Dag::Element& _element, Dag::Refine& _refine, I _depth)
	{
		std::vector<std::pair<Dag::Element*, Dag::Refine*>> refines{ {&_element, &_refine} };
		if (_depth > 0)
		{
			const I childDepth{ _depth - 1 };
			for (Dag::Element& child : _refine.children)
			{
				Dag::Refine& childRefine{ Refinement::Utils::prepare(_refine.forwardFi, _refine.firstVi, _refine.scheme) };
				std::vector<std::pair<Dag::Element*, Dag::Refine*>> childRefines{ prepareRecursive(child, childRefine, childDepth) };
				refines.insert(refines.end(), childRefines.begin(), childRefines.end());
			}
		}
		return refines;
	}

	void Refine::apply()
	{
		assert(mesher().shown(m_element));
		m_oldState = mesher().state();
		m_operation->parents.attach(m_element);
		for (const auto& [element, operation] : m_operations)
		{
			operation->parents.attach(*element);
			Refinement::Utils::apply(mesher(), *operation);
		}
		mesher().updateMesh();
	}

	void Refine::unapply()
	{
		for (const auto& [element, operation] : cpputils::range::of(m_operations).reverse())
		{
			operation->parents.detachAll(false);
		}
		mesher().show(m_element, true);
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	Refine::Refine(Dag::Element& _element, I _forwardFi, I _firstVi, Refinement::EScheme _scheme, I _depth)
		: m_element{ _element }, m_operation{ Refinement::Utils::prepare(_forwardFi, _firstVi, _scheme) }, m_depth{ _depth },
		m_operations{ prepareRecursive(_element, *m_operation, _depth - 1) }
	{
		assert(_depth >= 1 && _depth <= 3);
		assert(_forwardFi < 6);
		assert(_firstVi < 8);
		assert(Refinement::schemes.contains(_scheme));
	}

	const Dag::Element& Refine::element() const
	{
		return m_element;
	}

	const Dag::Refine& Refine::operation() const
	{
		return *m_operation;
	}

	I Refine::depth() const
	{
		return m_depth;
	}

}