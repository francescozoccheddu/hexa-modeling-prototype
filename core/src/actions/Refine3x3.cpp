#include <HMP/actions/Refine3x3.hpp>

#include <HMP/grid.hpp>
#include <HMP/Refinement/schemes.hpp>
#include <stdexcept>

namespace HMP::Actions
{

	Refine3x3::Refine3x3(unsigned int _pid)
		: m_pid(_pid)
	{}

	void Refine3x3::apply()
	{
		Grid& grid{ this->grid() };
		Dag::Element& element{ grid.element(m_pid) };
		if (element.children().any([](const Dag::Operation& _child) {return _child.primitive() != Dag::Operation::EPrimitive::Extrude; }))
		{
			throw std::logic_error{ "element has non-extrude child" };
		}
		Dag::Refine& operation{ *new Dag::Refine{} };
		m_operation = &operation;
		element.attachChild(operation);
		const Refinement::Scheme& scheme{ *Refinement::schemes.at(Refinement::EScheme::StandardRefinement) };
		const std::vector<Dag::Element*> children{ operation.attachChildren(scheme.polyCount()) };
		{
			for (std::size_t i{ 0 }; i < scheme.weights.size(); i++)
			{
				std::array<cinolib::vec3d, 8>& verts{ children[i]->vertices() };
				verts.fill(cinolib::vec3d{ 0,0,0 });

				for (std::size_t j{ 0 }; j < 8; j++)
				{
					const auto& vertWeights = scheme.weights[i][j];
					const auto& vertOffsets = scheme.offsets[i][j];
					for (std::size_t k{ 0 }; k < vertWeights.size(); k++)
					{
						verts[j] += vertWeights[k] * grid.mesh.poly_vert(m_pid, vertOffsets[k]);
					}
				}

				grid.addPoly(*children[i]);
			}
			grid.removePoly(m_pid);
			grid.update_mesh();
		}
	}

	void Refine3x3::unapply()
	{
		Grid& grid{ this->grid() };
		for (Dag::Element& child : m_operation->children())
		{
			grid.removePoly(child.pid());
		}
		grid.addPoly(m_operation->parents().single());
		delete m_operation;
	}

}