#include <HMP/Actions/Paste.hpp>

#include <HMP/Meshing/Utils.hpp>
#include <HMP/Actions/ExtrudeUtils.hpp>
#include <HMP/Dag/Utils.hpp>
#include <array>
#include <algorithm>
#include <utility>
#include <cassert>
#include <cpputils/range/of.hpp>
#include <cpputils/range/zip.hpp>

namespace HMP::Actions
{

	/*

	std::array<Vec, 3> basis(const Meshing::Mesher& _mesher, const Dag::Extrude& _extrude)
	{
		std::vector<Vec> newVerts;
		const HexVerts verts{ Meshing::Utils::verts(_mesher.mesh(), ExtrudeUtils::apply(_mesher, _extrude, newVerts)) };
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
		const Mat4 oldToNewHom{}; // TODO
		return Mat4::TRANS(origin(_mesher.mesh(), _target)) * oldToNewHom * Mat4::TRANS(-origin(_mesher.mesh(), _source));
	}

	void weld(const Meshing::Mesher& _mesher, Dag::Extrude& _extrude, const I _parentIndex, const std::array<I, 4>& _is)
	{
		Dag::Element& firstParent{ _extrude.parents.first() };
		const Id firstPid{ _mesher.elementToPid(firstParent) };
		const Id vid{ _mesher.mesh().poly_vert_id(firstPid, _extrude.vertOffset()) };
		Dag::Element& parent{ _extrude.parents()[_parentIndex] };
		const Id parentPid{ _mesher.elementToPid(parent) };
		const Id parentFid{ _mesher.mesh().poly_face_id(parentPid, _extrude.faceOffsets()[_parentIndex]) };
		const QuadVerts sourceVerts{ Meshing::Utils::verts(_mesher.mesh(), Meshing::Utils::pidFidVidsByFirstVid(_mesher.mesh(), parentPid, parentFid, vid)) };
		HexVerts& verts{ _extrude.children().single().vertices() };
		for (const auto& [sourceVert, targetI] : cpputils::range::zip(sourceVerts, _is))
		{
			verts[targetI] = sourceVert;
		}
	}

*/

	void Paste::apply()
	{
		/*
		for (Dag::Element* parent : m_elements)
		{
			m_operation->parents().attach(*parent);
		}
		if (!m_prepared)
		{
			m_prepared = true;
			const std::array<Vec, 3> oldBasis{ basis(mesher(), m_sourceOperation) };
			const std::array<Vec, 3> newBasis{ basis(mesher(), *m_operation) };
			const Mat3 oldToNorm{ Mat3{oldBasis[0], oldBasis[1], oldBasis[2]}.transpose() };
			const Mat3 newToNorm{ Mat3{newBasis[0], newBasis[1], newBasis[2]}.transpose() };
			const Mat3 normToNew{ newToNorm.inverse() };
			Dag::Utils::transform(*m_operation, Mat4::TRANS(-origin(mesher(), m_sourceOperation)));
			Dag::Utils::transform(*m_operation, normToNew * oldToNorm);
			Dag::Utils::transform(*m_operation, Mat4::TRANS(origin(mesher(), *m_operation)));
			if (m_sourceOperation.clockwise() != m_operation->clockwise())
			{
				for (Dag::Node* node : Dag::Utils::descendants(*m_operation))
				{
					if (node->isElement())
					{
						HexVerts& verts{ node->element().vertices() };
						const HexVerts oldVerts{ verts };
						verts[0] = oldVerts[0];
						verts[1] = oldVerts[3];
						verts[2] = oldVerts[2];
						verts[3] = oldVerts[1];
						verts[4] = oldVerts[4];
						verts[5] = oldVerts[7];
						verts[6] = oldVerts[6];
						verts[7] = oldVerts[5];
					}
				}
			}
			std::array<std::array<I, 4>, 3> indices{
				std::array<I, 4>{0,1,2,3},
				std::array<I, 4>{0,4,5,1},
				std::array<I, 4>{0,3,7,4}
			};
			if (m_operation->clockwise)
			{
				std::swap(indices[1], indices[2]);
			}
			for (I i{}; i < m_elements.size(); i++)
			{
				weld(mesher(), *m_operation, i, indices[i]);
			}
		}
		mesher().clear();
		Meshing::Utils::addLeafs(mesher(), *m_operation);
		mesher().updateMesh();
*/
	}

	void Paste::unapply()
	{
		/*
		Meshing::Utils::removeLeafs(mesher(), *m_operation, true);
		m_operation->parents.detachAll(false);
		mesher().updateMesh();
		*/
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
			case 4:
				m_operation->source = Dag::Extrude::ESource::Vertex;
				break;
			default:
				assert(false);
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