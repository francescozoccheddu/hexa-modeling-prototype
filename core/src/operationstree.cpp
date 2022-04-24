#include <HMP/operationstree.hpp>
#include <HMP/Refinement/schemes.hpp>

#include <assert.h>
#include <deque>
#include <fstream>
#include <iostream>
#include <cinolib/stl_container_utilities.h>
#include <HMP/Dag/Utils.hpp>

namespace HMP
{

	using namespace Dag;

	OperationsTree::OperationsTree()
	{
		root = new Element{};
	}

	OperationsTree::~OperationsTree()
	{
		if (root)
		{
			delete root;
		}
		root = nullptr;
	}


	void OperationsTree::add_children_to_refine(Refine& refine, Id num_children)
	{
		for (Id i = 0; i < num_children; i++)
		{
			refine.attachChild(*new Element{});
		}
	}

	Refine* OperationsTree::refine(Element& element, Id num_children)
	{
		if (!is_leaf(element)) return nullptr;

		Refine& refine{ *new Refine{} };
		for (Id i = 0; i < num_children; i++)
		{
			refine.attachChild(*new Element{});
		}

		element.attachChild(refine);

		return &refine;
	}


	Extrude* OperationsTree::extrude(Element& element, Id faceOffset)
	{
		if (!is_leaf(element)) return nullptr;
		assert(faceOffset <= 6);

		Extrude& extrude{ *new Extrude{} };
		extrude.faceOffset() = faceOffset;
		Element& child{ *new Element{} };
		extrude.attachChild(child);
		element.attachChild(extrude);

		return &extrude;
	}

	void OperationsTree::move(Element& element, Id faceOffset, const Vec& displacement)
	{
		if (!is_leaf(element)) return;

		assert(faceOffset <= 7);
		element.vertices()[faceOffset] = displacement;
	}


	Delete* OperationsTree::remove(Element& element)
	{
		if (!is_leaf(element)) return nullptr;

		Delete& deleteOp{ *new Delete{} };
		element.attachChild(deleteOp);
		return &deleteOp;
	}

	void OperationsTree::clear()
	{
		if (root)
		{
			delete root;
		}
		root = new Element{};
	}

	bool OperationsTree::is_leaf(const Element& element)
	{
		for (const auto& op : element.children())
		{
			if (op.primitive() == Operation::EPrimitive::Refine) return false;
		}
		return true;
	}

	bool OperationsTree::operation_is_leaf(const Operation& operation)
	{
		for (const auto& element : operation.children())
		{
			if (!element.isLeaf())
			{
				return false;
			}
		}
		return true;
	}

	bool OperationsTree::isRemoved(const Element& element)
	{
		for (const auto& op : element.children())
		{
			if (op.primitive() == Operation::EPrimitive::Delete) return false;
		}
		return true;

	}

	void OperationsTree::get_branch_elements(const Element& source, std::vector<const Element*>& elements, bool leaf_only)
	{
		for (const auto& operation : source.children())
		{
			get_branch_elements(operation, elements, leaf_only);
		}
	}

	void OperationsTree::get_branch_elements(const Operation& source, std::vector<const Element*>& elements, bool leaf_only)
	{

		std::deque<const Element*> queue;
		for (const auto& element : source.children())
		{
			queue.push_back(&element);
		}
		while (!queue.empty())
		{
			const auto& el = queue.front();
			queue.pop_front();
			for (const auto& operation : el->children())
			{
				for (const auto& element : operation.children())
				{
					queue.push_back(&element);
				}
			}
			if (leaf_only && is_leaf(*el)) elements.push_back(el);
		}
	}

	void OperationsTree::get_branch_operations(const Element& source, std::vector<const Operation*>& operations)
	{
		for (const auto& operation : source.children())
		{
			get_branch_operations(operation, operations);
		}
	}

	void OperationsTree::get_branch_operations(const Operation& source, std::vector<const Operation*>& operations)
	{
		operations.push_back(&source);
		std::deque<const Element*> queue;
		for (const auto& element : source.children())
		{
			queue.push_back(&element);
		}
		while (!queue.empty())
		{
			const auto& el = queue.front();
			queue.pop_front();
			for (const auto& operation : el->children())
			{
				for (const auto& element : operation.children())
				{
					queue.push_back(&element);
				}

				operations.push_back(&operation);
			}
		}
	}

	void OperationsTree::prune(Operation& operation)
	{
		delete &operation;
	}

	bool OperationsTree::check_compatibility(const Element& source, const Element& dest)
	{
		return true;

		std::vector<Id> extrude_offsets_source;
		std::vector<Id> extrude_offsets_dest;

		for (const auto& op : source.children())
		{
			if (op.primitive() == Operation::EPrimitive::Extrude)
			{
				const auto& ex = static_cast<const Extrude&>(op);
				extrude_offsets_source.push_back(ex.faceOffset());
			}
		}

		for (const auto& op : dest.children())
		{
			if (op.primitive() == Operation::EPrimitive::Extrude)
			{
				const auto& ex = static_cast<const Extrude&>(op);
				extrude_offsets_dest.push_back(ex.faceOffset());
			}
		}

		std::sort(extrude_offsets_source.begin(), extrude_offsets_source.end());
		std::sort(extrude_offsets_dest.begin(), extrude_offsets_dest.end());

		std::vector<Id> set_intersect;
		std::set_intersection(extrude_offsets_source.begin(), extrude_offsets_source.end(), extrude_offsets_dest.begin(), extrude_offsets_dest.end(), std::back_inserter(set_intersect));

		return set_intersect.size() == 0;
	}

	bool OperationsTree::merge(Element& source, Element& dest, std::vector<Operation*>* new_operations)
	{
		if (!is_leaf(dest) || !check_compatibility(source, dest)) return false;

		std::unordered_map<const Node*, Id> old_nodes;
		std::vector<Node*> old_nodes_vec;
		std::vector<Node*> nodes_vec;
		Id counter = 0;
		int num_new_ops = source.children().size();
		std::deque<Node*> queue;
		queue.push_back(&source);

		while (!queue.empty())
		{

			const auto& curr_node = queue.front();
			queue.pop_front();

			old_nodes_vec.push_back(curr_node);
			old_nodes[curr_node] = counter++;

			if (curr_node->isElement())
			{

				Element* new_el;

				if (nodes_vec.empty()) new_el = &dest;
				else new_el = new Element{};

				auto& el = static_cast<Element&>(*curr_node);
				nodes_vec.push_back(new_el);
				//new_el->displacements = el->displacements;

				for (auto& op : el.children())
				{
					if (!cinolib::CONTAINS(old_nodes, &op))
					{
						queue.push_back(&op);
					}
				}
			}
			else
			{
				auto& op = static_cast<Operation&>(*curr_node);

				switch (op.primitive())
				{
					case Operation::EPrimitive::Refine:
					{
						auto new_op = new Refine{};
						auto& old_ref = static_cast<Refine&>(op);

						new_op->scheme() = old_ref.scheme();
						new_op->vertices() = old_ref.vertices();

						if (old_ref.scheme() == Refinement::EScheme::Subdivide3x3) new_op->needsTopologyFix() = false;
						nodes_vec.push_back(new_op);
						if (num_new_ops-- > 0) new_operations->push_back(new_op);
						break;
					}
					case Operation::EPrimitive::Extrude:
					{
						auto new_op = new Extrude{};
						auto& old_ex = static_cast<Extrude&>(op);
						Id faceOffset = old_ex.faceOffset();
						new_op->faceOffset() = faceOffset;
						nodes_vec.push_back(new_op);
						if (num_new_ops-- > 0) new_operations->push_back(new_op);
						break;
					}
					case Operation::EPrimitive::Delete:

						auto new_op = new Delete{};
						auto& old_mv = static_cast<Delete&>(op);
						nodes_vec.push_back(new_op);
						if (num_new_ops-- > 0) new_operations->push_back(new_op);
						break;
				}

				for (auto& child : op.children())
				{
					queue.push_back(&child);
				}
			}

		}

		for (Id i = 0; i < old_nodes_vec.size(); i++)
		{

			auto& node = old_nodes_vec[i];
			auto& new_node = nodes_vec[i];

			if (node->isOperation())
			{
				auto& old_operation = node->operation();
				auto& new_operation = new_node->operation();

				for (const auto& father : old_operation.parents())
				{
					Id idx_father = old_nodes[&father];
					auto& new_father = nodes_vec[idx_father]->element();
					new_father.attachChild(new_operation);
				}
				for (auto& child : old_operation.children())
				{
					Id idx_child = old_nodes[&child];
					auto& new_child = nodes_vec[idx_child]->element();
					new_operation.attachChild(new_child);
				}
			}
		}

		return true;
	}

	int OperationsTree::op_element_offset(const Operation& op, const Element& element)
	{
		std::vector<Id> offsets;
		switch (op.primitive())
		{
			case Operation::EPrimitive::Extrude:
				return -1;
				break;
			default:
				return -1;
		}

		int off = -1;
		Id i = 0;

		for (const auto& parent : op.parents())
		{

			off = offsets[i++];
			if (parent == element) return off;
		}

		return -1;
	}

	bool OperationsTree::serialize(std::string filename)
	{
		using HMP::Dag::Utils::operator<<;
		std::ofstream file;
		file.open(filename);
		file << *root;
		file.close();
		return true;
	}

	bool OperationsTree::deserialize(std::string filename)
	{
		using HMP::Dag::Utils::operator>>;
		std::ifstream ifs;
		ifs.open(filename);
		if (ifs)
		{
			if (root)
			{
				delete root;
			}
			Node* root;
			ifs >> root;
			this->root = &root->element();
			return true;
		}
		return false;
	}

}