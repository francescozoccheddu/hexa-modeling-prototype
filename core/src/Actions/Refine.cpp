#include <HMP/Actions/Refine.hpp>

#include <HMP/Meshing/refinementSchemes.hpp>
#include <cpputils/collections/conversions.hpp>
#include <cpputils/collections/zip.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Refine::~Refine()
	{
		if (!applied())
		{
			m_operation.children().detachAll(true);
			delete& m_operation;
		}
	}

	void Refine::apply()
	{
		applyRefine(mesher(), m_element, m_operation);
	}

	void Refine::unapply()
	{
		unapplyRefine(mesher(), m_operation);
	}

	Dag::Refine& Refine::prepareRefine(Id _faceOffset, Meshing::ERefinementScheme _scheme)
	{
		Dag::Refine& refine{ *new Dag::Refine{} };
		refine.scheme() = _scheme;
		refine.faceOffset() = _faceOffset;
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(_scheme) };
		for (std::size_t i{ 0 }; i < refinement.polyCount(); i++)
		{
			refine.children().attach(*new Dag::Element{});
		}
		return refine;
	}

	void Refine::applyRefine(Meshing::Mesher& _mesher, Dag::Element& _element, Dag::Refine& _refine)
	{
		if (_element.children().cany([](const Dag::Operation& _child) {return _child.primitive() != Dag::Operation::EPrimitive::Extrude; }))
		{
			throw std::logic_error{ "element has non-extrude child" };
		}
		if (!_refine.isRoot())
		{
			throw std::logic_error{ "operation is not root" };
		}
		const Meshing::Refinement& refinement{ Meshing::refinementSchemes.at(_refine.scheme()) };
		if (refinement.polyCount() != _refine.children().size())
		{
			throw std::logic_error{ "wrong number of children" };
		}
		_element.children().attach(_refine);
		const Id pid{ _mesher.elementToPid(_element) };
		const Id fid{ _mesher.mesh().poly_face_id(pid, _refine.faceOffset()) };
		const PolyVerts source{ Meshing::Utils::polyVertsFromFace(_mesher.mesh(), pid, fid) };
		const std::vector<PolyVerts> polys{ refinement.apply(cpputils::collections::conversions::toVector(source)) };
		for (const auto& [child, verts] : cpputils::collections::zip(_refine.children(), polys))
		{
			child.vertices() = verts;
			_mesher.add(child);
		}
		_mesher.remove(_element);
	}

	void Refine::unapplyRefine(Meshing::Mesher& _mesher, Dag::Refine& _refine)
	{
		for (Dag::Element& child : _refine.children())
		{
			_mesher.remove(child);
		}
		_mesher.add(_refine.parents().single());
		_refine.parents().detachAll(false);
	}

	Refine::Refine(Dag::Element& _element, Id _faceOffset, Meshing::ERefinementScheme _scheme)
		: m_element{ _element }, m_operation{ prepareRefine(_faceOffset, _scheme) }
	{}

}