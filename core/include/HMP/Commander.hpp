#pragma once

#include <deque>
#include <HMP/Dag/Element.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>
#include <cpputils/collections/DereferenceIterable.hpp>
#include <HMP/Meshing/Mesher.hpp>

namespace HMP
{

	class Project;

	class Commander final : public cpputils::mixins::ReferenceClass
	{

	private:

		class Action;

		class ActionBase : public cpputils::mixins::ReferenceClass
		{

		private:

			friend class Action;

			Commander* m_commander;
			bool m_applied;

			ActionBase();

			void ensureAttached() const;
			void attach(Commander& _commander);

			void prepareAndApply();
			void prepareAndUnapply();

		protected:

			virtual ~ActionBase() = default;
			
			Meshing::Mesher& mesher();
			const Meshing::Mesher& mesher() const;

			const Dag::Element* root() const;
			Dag::Element*& root();

			virtual void apply() = 0;
			virtual void unapply() = 0;

		};

		class Stack;

		class StackBase : public cpputils::mixins::ReferenceClass, public cpputils::collections::DereferenceIterable<std::deque<Action*>, const Action&, const Action&>
		{

		private:

			friend class Stack;

			std::deque<Action*> m_data;
			std::size_t m_limit;

			StackBase();

			Action& pop();
			void push(Action& _action);

		public:

			std::size_t limit() const;
			void limit(std::size_t _count);
			void removeOldest(std::size_t _count);
			void keepLatest(std::size_t _count);
			void clear();

		};

	public:

		class Action : public ActionBase
		{

		private:

			friend class Commander;

			using ActionBase::ensureAttached;
			using ActionBase::attach;
			using ActionBase::prepareAndApply;
			using ActionBase::prepareAndUnapply;

		public:

			using ActionBase::ActionBase;

		};

		class Stack final : public StackBase
		{

		private:

			friend class Commander;

			using StackBase::StackBase;
			using StackBase::pop;
			using StackBase::push;

		};

	private:

		Project& m_project;
		Stack m_unapplied{}, m_applied{};

	public:

		Commander(Project& _project);
		~Commander();

		void apply(Action& _action);

		void undo();
		void redo();

		bool canUndo() const;
		bool canRedo() const;

		Stack& unapplied();
		const Stack& unapplied() const;

		Stack& applied();
		const Stack& applied() const;

	};

}