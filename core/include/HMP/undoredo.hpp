#pragma once

#include <stack>
#include <list>
#include <memory>
#include <iostream>
#include <HMP/operationstree.hpp>

namespace HMP
{

	class Action
	{

	public:
		virtual void execute() { std::cout << "calling execute of base class" << std::endl; }
		virtual void undo() { std::cout << "calling undo of base class" << std::endl; }

	};

	class CommandManager
	{
	public:

		static CommandManager& get_instance()
		{

			static CommandManager instance;
			return instance;
		}

		CommandManager(CommandManager const&) = delete;
		void operator=(CommandManager const&) = delete;


		void execute(std::list<std::shared_ptr<Action> > actions);
		bool undo();
		bool redo();
		void clear();
		void clear_undo();
		void clear_redo();

		void collapse_last_n_actions(unsigned int n);


	private:

		CommandManager();

		std::stack<std::list<std::shared_ptr<Action>>> undo_stack;
		std::stack<std::list<std::shared_ptr<Action>>> redo_stack;

	};

}