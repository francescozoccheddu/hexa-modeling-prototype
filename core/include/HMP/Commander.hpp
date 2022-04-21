#pragma once

#include <deque>

namespace HMP
{

	class Grid;

	class Commander final
	{

	public:

		class Action
		{

		private:

			friend class Commander;
			Commander* m_commander;
			bool m_applied;

			void ensureAttached() const;
			void attach(Commander& _commander);

			void prepareAndApply();
			void prepareAndUnapply();

		protected:

			Grid& grid();
			const Grid& grid() const;

			virtual ~Action() = default;
			virtual void apply() = 0;
			virtual void unapply() = 0;

		};

	private:

		Grid& m_grid;
		std::deque<Action*> m_applied{};
		std::deque<Action*> m_unapplied{};

	public:

		Commander(Grid& _grid);
		~Commander();

		void apply(Action& _action);
		void undo();
		void redo();

		void clear();
		void clearUndo();
		void clearRedo();
		void limitUndo(std::size_t _count);
		void limitRedo(std::size_t _count);

		bool canUndo() const;
		bool canRedo() const;
		std::size_t undoCount() const;
		std::size_t redoCount() const;

		bool operator==(const Commander& _other) const;
		bool operator!=(const Commander& _other) const = default;

	};

}