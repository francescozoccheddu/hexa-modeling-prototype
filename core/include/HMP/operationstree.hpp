#pragma once

#include <vector>
#include <list>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <HMP/utils.hpp>
#include <regex>

namespace HMP
{

	enum SchemeType : unsigned int { STDREF = 0, FACESCM = 1, EDGESCM = 2, FACEREF = 3 };
	enum Primitive { REFINE, EXTRUDE, REMOVE };
	enum NodeType { ELEMENT, OPERATION };

	struct Operation;
	struct Element;

	struct Node
	{
		NodeType type;
		std::list<std::shared_ptr<Node>> parents;
	};

	struct Operation : Node
	{
		Primitive primitive;
		std::vector<std::shared_ptr<Element>> children;
		std::list<std::shared_ptr<Operation>> depends_on;
		std::list<std::shared_ptr<Operation>> dependency_of;
		bool is_user_defined = true;
		Operation() { type = OPERATION; }
		virtual std::string toString() = 0;
	};

	struct Element : Node
	{

		Element() { type = ELEMENT; }
		std::list<std::shared_ptr<Operation>> operations;
		std::vector<cinolib::vec3d> displacements = std::vector<cinolib::vec3d>(8);
		std::string toString()
		{
			std::string to_return = "EL";
			return to_return;
		}
	};

	struct Refine : Operation
	{
		Refine() { primitive = REFINE; }
		SchemeType scheme_type = STDREF;
		std::vector<unsigned int> vert_map = { 0,1,2,3,4,5,6,7 };
		std::string toString()
		{
			return "R" + std::to_string(scheme_type) +
				"<" + std::to_string(vert_map[0]) + std::to_string(vert_map[1]) +
				std::to_string(vert_map[2]) + std::to_string(vert_map[3]) +
				std::to_string(vert_map[4]) + std::to_string(vert_map[5]) +
				std::to_string(vert_map[6]) + std::to_string(vert_map[7]) + ">";
		}

		bool needs_fix_topology = true;
	};

	struct Extrude : Operation
	{
		unsigned int offset;
		Extrude() { primitive = EXTRUDE; }
		std::string toString() { return "E" + std::to_string(offset); }

	};

	struct Remove : Operation
	{
		Remove() { primitive = REMOVE; }
		std::string toString() { return "DL"; }
	};


	class OperationsTree
	{
	public:
		OperationsTree();
		~OperationsTree();

		//Attributes
		std::shared_ptr<Element> root;

		//Methods
		std::shared_ptr<Refine> refine(std::shared_ptr<Element> element, unsigned int num_children = 27);
		std::shared_ptr<Extrude> extrude(std::shared_ptr<Element> element, unsigned int offset);
		void move(std::shared_ptr<Element> element, unsigned int offset, const cinolib::vec3d& displacement);
		std::shared_ptr<Remove> remove(std::shared_ptr<Element> element);
		void clear();
		bool is_leaf(const std::shared_ptr<Element>& element);
		bool operation_is_leaf(const std::shared_ptr<Operation>& operation);
		bool isRemoved(const std::shared_ptr<Element>& element);
		bool check_compatibility(const std::shared_ptr<Element>& source, const std::shared_ptr<Element>& dest);
		bool merge(const std::shared_ptr<Element>& source, const std::shared_ptr<Element>& dest, std::list<std::shared_ptr<Operation> >* new_operations = nullptr);
		bool serialize(std::string filename);
		bool deserialize(std::string filename);
		void toString(std::shared_ptr<Node> root, std::string& data);
		int op_element_offset(const std::shared_ptr<Operation>& op, const std::shared_ptr<Element>& element);
		void add_children_to_refine(std::shared_ptr<Refine>& refine, unsigned int num_children);
		void get_branch_elements(const std::shared_ptr<Element>& source, std::vector<std::shared_ptr<Element>>& elements, bool leaf_only = true);
		void get_branch_elements(const std::shared_ptr<Operation>& source, std::vector<std::shared_ptr<Element>>& elements, bool leaf_only = true);
		void get_branch_operations(const std::shared_ptr<Element>& source, std::vector<std::shared_ptr<Operation>>& operations);
		void get_branch_operations(const std::shared_ptr<Operation>& source, std::vector<std::shared_ptr<Operation>>& operations);
		void prune(const std::shared_ptr<Operation>& operation);


		std::map<std::vector<unsigned int>, std::shared_ptr<Element>> vids2element;
		std::unordered_map<std::shared_ptr<Element>, std::vector<unsigned int>> element2vids;


	private:

	};

}