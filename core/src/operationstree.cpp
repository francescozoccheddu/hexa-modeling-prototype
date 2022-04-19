#include <HMP/operationstree.hpp>

#include <assert.h>
#include <deque>
#include <cinolib/stl_container_utilities.h>

namespace HMP
{

	OperationsTree::OperationsTree()
	{
		root = std::shared_ptr<Element>(new Element);
	}

	OperationsTree::~OperationsTree()
	{

	}


	void OperationsTree::add_children_to_refine(std::shared_ptr<Refine>& refine, unsigned int num_children)
	{

		for (unsigned int i = 0; i < num_children; i++)
		{
			refine->children.push_back(std::shared_ptr<Element>(new Element));
			refine->children.back()->parents.push_back(refine);
		}

	}

	std::shared_ptr<Refine> OperationsTree::refine(std::shared_ptr<Element> element, unsigned int num_children)
	{
		if (!is_leaf(element)) return nullptr;

		auto refine = std::shared_ptr<Refine>(new Refine);
		for (unsigned int i = 0; i < num_children; i++)
		{
			refine->children.push_back(std::shared_ptr<Element>(new Element));
			refine->children.back()->parents.push_back(refine);
		}

		element->operations.push_back(refine);
		refine->parents.push_back(element);

		return refine;
	}


	std::shared_ptr<Extrude> OperationsTree::extrude(std::shared_ptr<Element> element, unsigned int offset)
	{
		if (!is_leaf(element)) return nullptr;
		assert(offset <= 6);

		auto extrude = std::shared_ptr<Extrude>(new Extrude);
		extrude->offset = offset;
		auto child = std::shared_ptr<Element>(new Element);
		extrude->children.push_back(child);
		child->parents.push_back(extrude);

		element->operations.push_back(extrude);
		extrude->parents.push_back(element);

		//ADJS
		//child->adjs.push_back(element);
		//element->adjs.push_back(child);

		return extrude;
	}

	void OperationsTree::move(std::shared_ptr<Element> element, unsigned int offset, const cinolib::vec3d& displacement)
	{
		if (!is_leaf(element)) return;

		assert(offset <= 7);
		element->displacements[offset] = displacement;
	}


	std::shared_ptr<Remove> OperationsTree::remove(std::shared_ptr<Element> element)
	{
		if (!is_leaf(element)) return nullptr;

		auto remove = std::shared_ptr<Remove>(new Remove);
		element->operations.push_back(remove);
		remove->parents.push_back(element);
		return remove;
	}

	void OperationsTree::clear()
	{
		root.reset();
		root = std::shared_ptr<Element>(new Element);
	}

	bool OperationsTree::is_leaf(const std::shared_ptr<Element>& element)
	{
		for (const auto& op : element->operations)
		{
			if (op->primitive == REFINE) return false;
		}
		return true;
	}

	bool OperationsTree::operation_is_leaf(const std::shared_ptr<Operation>& operation)
	{
		for (const auto& element : operation->children)
		{
			if (!element->operations.empty())
			{
				return false;
			}
		}
		return true;
	}

	bool OperationsTree::isRemoved(const std::shared_ptr<Element>& element)
	{
		for (const auto& op : element->operations)
		{
			if (op->primitive == REMOVE) return false;
		}
		return true;

	}

	void OperationsTree::get_branch_elements(const std::shared_ptr<Element>& source, std::vector<std::shared_ptr<Element>>& elements, bool leaf_only)
	{
		for (const auto& operation : source->operations)
		{
			get_branch_elements(operation, elements, leaf_only);
		}
	}

	void OperationsTree::get_branch_elements(const std::shared_ptr<Operation>& source, std::vector<std::shared_ptr<Element>>& elements, bool leaf_only)
	{

		std::deque<std::shared_ptr<Element>> queue;
		for (const auto& element : source->children)
		{
			queue.push_back(element);
		}
		while (!queue.empty())
		{
			const auto& el = queue.front();
			queue.pop_front();
			for (const auto& operation : el->operations)
			{
				for (const auto& element : operation->children)
				{
					queue.push_back(element);
				}
			}
			if (leaf_only && is_leaf(el)) elements.push_back(el);
		}
	}

	void OperationsTree::get_branch_operations(const std::shared_ptr<Element>& source, std::vector<std::shared_ptr<Operation> >& operations)
	{
		for (const auto& operation : source->operations)
		{
			get_branch_operations(operation, operations);
		}
	}

	void OperationsTree::get_branch_operations(const std::shared_ptr<Operation>& source, std::vector<std::shared_ptr<Operation> >& operations)
	{
		operations.push_back(source);
		std::deque<std::shared_ptr<Element>> queue;
		for (const auto& element : source->children)
		{
			queue.push_back(element);
		}
		while (!queue.empty())
		{
			const auto& el = queue.front();
			queue.pop_front();
			for (const auto& operation : el->operations)
			{
				for (const auto& element : operation->children)
				{
					queue.push_back(element);
				}

				operations.push_back(operation);
			}
		}
	}

	void OperationsTree::prune(const std::shared_ptr<Operation>& operation)
	{
		for (auto& node : operation->parents)
		{
			auto parent = std::static_pointer_cast<Element>(node);
			parent->operations.remove(operation);

		}
	}

	bool OperationsTree::check_compatibility(const std::shared_ptr<Element>& source, const std::shared_ptr<Element>& dest)
	{
		return true;

		std::vector<unsigned int> extrude_offsets_source;
		std::vector<unsigned int> extrude_offsets_dest;

		for (const auto& op : source->operations)
		{
			if (op->primitive == EXTRUDE)
			{
				const auto& ex = std::static_pointer_cast<Extrude>(op);
				extrude_offsets_source.push_back(ex->offset);
			}
		}

		for (const auto& op : dest->operations)
		{
			if (op->primitive == EXTRUDE)
			{
				const auto& ex = std::static_pointer_cast<Extrude>(op);
				extrude_offsets_dest.push_back(ex->offset);
			}
		}

		std::sort(extrude_offsets_source.begin(), extrude_offsets_source.end());
		std::sort(extrude_offsets_dest.begin(), extrude_offsets_dest.end());

		std::vector<unsigned int> set_intersect;
		std::set_intersection(extrude_offsets_source.begin(), extrude_offsets_source.end(), extrude_offsets_dest.begin(), extrude_offsets_dest.end(), std::back_inserter(set_intersect));

		return set_intersect.size() == 0;
	}

	bool OperationsTree::merge(const std::shared_ptr<Element>& source, const std::shared_ptr<Element>& dest, std::list<std::shared_ptr<Operation>>* new_operations)
	{
		if (!is_leaf(dest) || !check_compatibility(source, dest)) return false;

		std::unordered_map<std::shared_ptr<Node>, unsigned int> old_nodes;
		std::vector<std::shared_ptr<Node>> old_nodes_vec;
		std::vector<std::shared_ptr<Node>> nodes_vec;
		unsigned int counter = 0;
		int num_new_ops = source->operations.size();
		std::deque<std::shared_ptr<Node>> queue;
		queue.push_back(source);

		while (!queue.empty())
		{

			const auto& curr_node = queue.front();
			queue.pop_front();

			old_nodes_vec.push_back(curr_node);
			old_nodes[curr_node] = counter++;

			if (curr_node->type == ELEMENT)
			{

				std::shared_ptr<Element> new_el;

				if (nodes_vec.empty()) new_el = dest;
				else new_el = std::shared_ptr<Element>(new Element);

				const auto& el = std::static_pointer_cast<Element>(curr_node);
				nodes_vec.push_back(new_el);
				//new_el->displacements = el->displacements;

				for (const auto& op : el->operations)
				{
					if (!cinolib::CONTAINS(old_nodes, op))
					{
						queue.push_back(op);
					}
				}
			}
			else
			{
				auto op = std::static_pointer_cast<Operation>(curr_node);

				switch (op->primitive)
				{
					case REFINE:
					{
						auto new_op = std::shared_ptr<Refine>(new Refine);
						auto old_ref = std::static_pointer_cast<Refine>(op);

						new_op->scheme_type = old_ref->scheme_type;
						new_op->vert_map = old_ref->vert_map;

						if (old_ref->scheme_type == STDREF) new_op->needs_fix_topology = false;
						nodes_vec.push_back(new_op);
						if (num_new_ops-- > 0) new_operations->push_back(new_op);
						break;
					}
					case EXTRUDE:
					{
						auto new_op = std::shared_ptr<Extrude>(new Extrude);
						auto old_ex = std::static_pointer_cast<Extrude>(op);
						unsigned int offset = old_ex->offset;
						new_op->offset = offset;
						nodes_vec.push_back(new_op);
						if (num_new_ops-- > 0) new_operations->push_back(new_op);
						break;
					}
					case REMOVE:

						auto new_op = std::shared_ptr<Remove>(new Remove);
						auto old_mv = std::static_pointer_cast<Remove>(op);
						nodes_vec.push_back(new_op);
						if (num_new_ops-- > 0) new_operations->push_back(new_op);
						break;
				}

				for (const auto& child : op->children)
				{
					queue.push_back(child);
				}
			}

		}

		for (unsigned int i = 0; i < old_nodes_vec.size(); i++)
		{

			const auto& node = old_nodes_vec[i];
			const auto& new_node = nodes_vec[i];

			if (node->type == OPERATION)
			{
				auto old_operation = std::static_pointer_cast<Operation>(node);
				auto new_operation = std::static_pointer_cast<Operation>(new_node);

				for (const auto& father : old_operation->parents)
				{
					unsigned int idx_father = old_nodes[father];
					const auto& new_father = std::static_pointer_cast<Element>(nodes_vec[idx_father]);
					new_operation->parents.push_back(new_father);
					new_father->operations.push_back(new_operation);


				}
				for (const auto& child : old_operation->children)
				{
					unsigned int idx_child = old_nodes[child];
					const auto& new_child = std::static_pointer_cast<Element>(nodes_vec[idx_child]);
					new_operation->children.push_back(new_child);
					new_child->parents.push_back(new_operation);
				}
			}
		}

		return true;
	}

	void OperationsTree::toString(std::shared_ptr<Node> root, std::string& data)
	{

		data.clear();

		std::unordered_map<std::shared_ptr<Node>, unsigned int> nodes;
		std::vector<std::shared_ptr<Node>> nodes_vec;
		std::deque<std::shared_ptr<Node>> queue;

		unsigned int counter = 0;
		queue.push_back(root);

		while (!queue.empty())
		{

			auto curr_node = queue.front();
			queue.pop_front();

			nodes[curr_node] = counter++;
			nodes_vec.push_back(curr_node);

			if (curr_node->type == ELEMENT)
			{
				auto element = std::static_pointer_cast<Element>(curr_node);
				for (const auto& op : element->operations)
				{
					if (!cinolib::CONTAINS(nodes, op))
					{
						queue.push_back(op);
					}
				}
			}
			else
			{
				auto operation = std::static_pointer_cast<Operation>(curr_node);
				for (const auto& child : operation->children)
				{
					queue.push_back(child);
				}
			}
		}

		data += "Nodes:\n";
		for (const auto& node : nodes_vec)
		{
			if (node->type == ELEMENT)
			{
				auto element = std::static_pointer_cast<Element>(node);
				data += element->toString() + "[";
				if (is_leaf(element))
				{
					for (unsigned int i = 0; i < 8; i++)
					{
						data += std::to_string(element->displacements[i].x()) + "," +
							std::to_string(element->displacements[i].y()) + "," +
							std::to_string(element->displacements[i].z()) + "|";
					}
				}
				data += "]\n";
			}
			else
			{
				auto operation = std::static_pointer_cast<Operation>(node);
				data += operation->toString() + "\n";
			}
		}
		data += "Arcs:\n";
		for (const auto& node : nodes_vec)
		{
			if (node->type == OPERATION)
			{
				auto operation = std::static_pointer_cast<Operation>(node);
				unsigned int idx_op = nodes[node];
				data += std::to_string(idx_op) + ",";
				for (const auto& father : operation->parents)
				{
					data += std::to_string(nodes[father]) + "-";
				}
				data += ",";
				for (const auto& child : operation->children)
				{
					data += std::to_string(nodes[child]) + "-";
				}
				data += "\n";
			}
		}

	}

	int OperationsTree::op_element_offset(const std::shared_ptr<Operation>& op, const std::shared_ptr<Element>& element)
	{
		std::vector<unsigned int> offsets;
		switch (op->primitive)
		{
			case EXTRUDE:
				return -1;
				break;
			default:
				return -1;
		}

		int off = -1;
		unsigned int i = 0;

		for (const auto& parent : op->parents)
		{

			off = offsets[i++];
			if (parent == element) return off;
		}

		return -1;
	}

	bool OperationsTree::serialize(std::string filename)
	{
		std::string data = "";
		toString(root, data);
		std::ofstream file;
		file.open(filename);
		file << data;
		file.close();

		return true;
	}

	bool OperationsTree::deserialize(std::string filename)
	{
		std::string data = "";
		std::ifstream ifs;

		auto string2vec3d = [](std::string& string) {

			std::stringstream ss(string);
			std::string split;

			unsigned int i = 0;
			cinolib::vec3d to_return(0.0);
			while (std::getline(ss, split, ','))
			{
				to_return[i++] = std::stod(split);
			}

			return to_return;
		};

		ifs.open(filename);
		if (ifs)
		{
			std::stringstream buffer;
			buffer << ifs.rdbuf();
			std::string line;
			std::regex regex("E[0-5]|R[0-5]|DL");

			std::vector<std::shared_ptr<Node>> nodes_vec;

			while (std::getline(buffer, line))
			{
				std::string code = line.substr(0, 2);
				if (code == "EL")
				{
					std::shared_ptr<Element> el;
					if (nodes_vec.empty()) el = this->root;
					else el = std::shared_ptr<Element>(new Element);

					unsigned int idx1 = line.find("[");
					unsigned int idx2 = line.find("]");

					std::string displacement_string = line.substr(idx1 + 1, idx2 - (idx1 + 1));

					std::stringstream ss(displacement_string);
					std::string split;

					unsigned int i = 0;
					while (std::getline(ss, split, '|'))
					{
						if (split.empty()) continue;
						el->displacements[i++] = string2vec3d(split);
					}

					nodes_vec.push_back(el);
				}
				else if (std::regex_match(code, regex))
				{

					std::shared_ptr<Operation> op;
					if (code[0] == 'E')
					{
						op = std::shared_ptr<Extrude>(new Extrude);
						char* c = &code[1];
						auto ex = std::static_pointer_cast<Extrude>(op);
						unsigned int offset = std::atoi(c);
						ex->offset = offset;
					}
					else if (code[0] == 'R')
					{
						op = std::shared_ptr<Refine>(new Refine);
						char* c = &code[1];
						auto ref = std::static_pointer_cast<Refine>(op);
						unsigned int type = std::atoi(c);
						ref->scheme_type = static_cast<SchemeType>(type);
						ref->is_user_defined = true;
						ref->needs_fix_topology = false;
						for (unsigned int i = 0; i < 8; i++) ref->vert_map[i] = line[i + 3] - 48;
					}

					else if (code[0] == 'D') op = std::shared_ptr<Remove>(new Remove);

					nodes_vec.push_back(op);

				}

				if (line == "Arcs:") break;
			}
			while (std::getline(buffer, line))
			{
				std::stringstream ss(line);
				std::string split;
				unsigned int i = 0;
				unsigned int op_idx = 0;
				std::vector<unsigned int> op_fathers;
				std::vector<unsigned int> op_children;
				while (std::getline(ss, split, ','))
				{
					if (i == 0) op_idx = std::stoi(split);
					else if (i == 1)
					{
						std::stringstream subss(split);
						std::string subsplit;
						while (std::getline(subss, subsplit, '-'))
						{
							op_fathers.push_back(std::stoi(subsplit));
						}
					}
					else
					{
						std::stringstream subss(split);
						std::string subsplit;
						while (std::getline(subss, subsplit, '-'))
						{
							op_children.push_back(std::stoi(subsplit));
						}
					}
					i++;
				}

				auto op = std::static_pointer_cast<Operation>(nodes_vec[op_idx]);
				for (unsigned int idx : op_fathers)
				{
					op->parents.push_back(nodes_vec[idx]);
				}
				for (const auto& father : op->parents)
				{
					auto el = std::static_pointer_cast<Element>(father);
					el->operations.push_back(op);
				}
				for (unsigned int idx : op_children)
				{
					auto el = std::static_pointer_cast<Element>(nodes_vec[idx]);
					op->children.push_back(el);
					el->parents.push_back(op);
				}
			}

			return true;
		}


		return false;
	}

}