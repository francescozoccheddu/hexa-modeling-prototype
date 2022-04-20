#pragma once

#include <vector>
#include <list>
#include <string>
#include <HMP/utils.hpp>
#include <HMP/Refinement/schemes.hpp>
#include <HMP/Dag/Element.hpp>
#include <HMP/Dag/Delete.hpp>
#include <HMP/Dag/Extrude.hpp>
#include <HMP/Dag/Refine.hpp>

namespace HMP
{

	class OperationsTree
	{
	public:
		OperationsTree();
		~OperationsTree();

		//Attributes
		Dag::Element* root{};

		//Methods
		Dag::Refine* refine(Dag::Element& element, unsigned int num_children = 27);
		Dag::Extrude* extrude(Dag::Element& element, unsigned int offset);
		void move(Dag::Element& element, unsigned int offset, const cinolib::vec3d& displacement);
		Dag::Delete* remove(Dag::Element& element);
		void clear();
		bool is_leaf(const Dag::Element& element);
		bool operation_is_leaf(const Dag::Operation& operation);
		bool isRemoved(const Dag::Element& element);
		bool check_compatibility(const Dag::Element& source, const Dag::Element& dest);
		bool merge(Dag::Element& source, Dag::Element& dest, std::vector<Dag::Operation*>* new_operations = nullptr);
		bool serialize(std::string filename);
		bool deserialize(std::string filename);
		int op_element_offset(const Dag::Operation& op, const Dag::Element& element);
		void add_children_to_refine(Dag::Refine& refine, unsigned int num_children);
		void get_branch_elements(const Dag::Element& source, std::vector<const Dag::Element*>& elements, bool leaf_only = true);
		void get_branch_elements(const Dag::Operation& source, std::vector<const Dag::Element*>& elements, bool leaf_only = true);
		void get_branch_operations(const Dag::Element& source, std::vector<const Dag::Operation*>& operations);
		void get_branch_operations(const Dag::Operation& source, std::vector<const Dag::Operation*>& operations);
		void prune(Dag::Operation& operation);

	};

}