#include <HMP/Actions/Paste.hpp>

#include <cpputils/unreachable.hpp>
#include <HMP/Meshing/Utils.hpp>
#include <HMP/Actions/ExtrudeUtils.hpp>
#include <HMP/Dag/Utils.hpp>
#include <HMP/Dag/Refine.hpp>
#include <array>
#include <algorithm>
#include <utility>
#include <cassert>
#include <cpputils/range/of.hpp>
#include <cpputils/range/zip.hpp>
#include <cinolib/geometry/vec_mat_utils.h>
#include <unordered_map>

namespace HMP::Actions
{

	std::array<Vec, 3> basis(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude)
	{
		std::vector<Vec> newVerts;
		const HexVerts verts{ Meshing::Utils::verts(_mesher.mesh(), ExtrudeUtils::apply(_mesher, _extrude, newVerts), newVerts) };
		std::array<I, 3> indices{ 1,4,3 };
		if (_extrude.clockwise)
		{
			std::reverse(indices.begin(), indices.end());
		}
		return cpputils::range::of(indices).map([&](I _i) { return (verts[_i] - verts[0]).normalized(); }).toArray();
	}

	Vec origin(const Meshing::Mesher::Mesh& _mesh, const Dag::Extrude& _extrude)
	{
		Id vid{ _extrude.parents.first().vids[_extrude.firstVi] };
		return _mesh.vert(vid);
	}

	Mat4 transformMat(const Meshing::Mesher& _mesher, const Dag::Extrude& _source, const Dag::Extrude& _target)
	{
		const std::array<Vec, 3> oldBasis{ basis(_mesher, _source) };
		const std::array<Vec, 3> newBasis{ basis(_mesher, _target) };
		const Mat3 oldToNorm{ Mat3{oldBasis[0], oldBasis[1], oldBasis[2]}.transpose() };
		const Mat3 newToNorm{ Mat3{newBasis[0], newBasis[1], newBasis[2]}.transpose() };
		const Mat3 oldToNew{ newToNorm.inverse() * oldToNorm };
		return Mat4::TRANS(origin(_mesher.mesh(), _target)) * Mat4::HOMOGENEOUS(oldToNew) * Mat4::TRANS(-origin(_mesher.mesh(), _source));
	}

	std::unordered_map<Id, Id> weld(const Dag::Extrude& _extrude, const I _parentIndex, const std::array<I, 4>& _is)
	{
		const Dag::Element& firstParent{ _extrude.parents.first() };
		const Id firstVid{ firstParent.vids[_extrude.firstVi] };
		const Dag::Element& parent{ _extrude.parents[_parentIndex] };
		const QuadVertIds parentVids{ Meshing::Utils::align(Meshing::Utils::fiVids(parent.vids, _extrude.fis[_parentIndex]), firstVid) };
		std::unordered_map<Id, Id> vidsMap{};
		vidsMap.reserve(4);
		for (const auto& [newVid, targetI] : cpputils::range::zip(parentVids, _is))
		{
			const Id oldVid{ _extrude.children.single().vids[targetI] };
			vidsMap.emplace(oldVid, newVid);
		}
		return vidsMap;
	}

	void Paste::apply()
	{
		m_oldState = mesher().state();
		for (Dag::Element* parent : m_elements)
		{
			m_operation->parents.attach(*parent);
		}
		if (!m_prepared)
		{
			m_prepared = true;
			std::array<std::array<I, 4>, 3> indices{
				std::array<I, 4>{0,1,2,3},
				std::array<I, 4>{0,4,5,1},
				std::array<I, 4>{0,3,7,4}
			};
			if (m_operation->clockwise)
			{
				std::swap(indices[1], indices[2]);
			}
			const Meshing::Mesher::Mesh& mesh{ mesher().mesh() };
			const Id numVerts{ mesh.num_verts() };
			const Mat4 transform{ transformMat(mesher(), m_sourceOperation, *m_operation) };
			if (m_sourceOperation.clockwise != m_operation->clockwise)
			{
				for (Dag::Node* node : Dag::Utils::descendants(*m_operation))
				{
					if (node->isElement())
					{
						Dag::Element& element{ node->element() };
						element.vids = Meshing::Utils::reverse(element.vids);
					}
				}
			}
			std::unordered_map<Id, Id> vidMap{};
			for (I i{}; i < m_elements.size(); i++)
			{
				vidMap.merge(weld(*m_operation, i, indices[i]));
			}
			for (Dag::Node* node : Dag::Utils::descendants(*m_operation))
			{
				if (node->isElement())
				{
					node->element().pid = noId;
					for (Id& vid : node->element().vids)
					{
						const auto it{ vidMap.find(vid) };
						if (it != vidMap.end())
						{
							vid = it->second;
						}
						else
						{
							const Id oldVid{ vid };
							vid = numVerts + toId(m_newVerts.size());
							vidMap.emplace_hint(it, oldVid, vid);
							m_newVerts.push_back(transform * mesh.vert(oldVid));
						}
					}
				}
			}
			for (Dag::Node* node : Dag::Utils::descendants(*m_operation))
			{
				if (node->isOperation())
				{
					Dag::Operation& operation{ node->operation() };
					if (operation.primitive == Dag::Operation::EPrimitive::Refine)
					{
						Dag::Refine& refine{ static_cast<Dag::Refine&>(operation) };
						for (Id& vid : refine.surfVids)
						{
							vid = vidMap.at(vid);
						}
					}
				}
			}
		}
		Meshing::Utils::addTree(mesher(), *m_operation, m_newVerts);
		mesher().updateMesh();
	}

	void Paste::unapply()
	{
		m_operation->parents.detachAll(false);
		mesher().restore(m_oldState);
		mesher().updateMesh();
	}

	Paste::Paste(const cpputils::collections::FixedVector<Dag::Element*, 3>& _elements, const cpputils::collections::FixedVector<I, 3>& _fis, I _firstVi, bool _clockwise, const Dag::Extrude& _source)
		: m_elements{ _elements }, m_operation{ static_cast<Dag::Extrude&>(Dag::Utils::clone(_source)) }, m_sourceOperation{ _source }
	{
		m_operation->fis = _fis;
		m_operation->clockwise = _clockwise;
		m_operation->firstVi = _firstVi;
		switch (m_elements.size())
		{
			case 1:
				m_operation->source = Dag::Extrude::ESource::Face;
				break;
			case 2:
				m_operation->source = Dag::Extrude::ESource::Edge;
				break;
			case 3:
				m_operation->source = Dag::Extrude::ESource::Vertex;
				break;
			default:
				cpputils::unreachable();
		}
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