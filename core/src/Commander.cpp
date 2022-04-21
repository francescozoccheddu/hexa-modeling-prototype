#include <HMP/Commander.hpp>

#include <HMP/grid.hpp>
#include <stdexcept>

namespace HMP
{

	// Commander::Action

	void Commander::Action::ensureAttached() const
	{
		if (!m_commander)
		{
			throw std::logic_error{ "not attached" };
		}
	}

	void Commander::Action::attach(Commander& _commander)
	{
		if (m_commander)
		{
			throw std::logic_error{ "already attached" };
		}
		m_commander = &_commander;
	}

	void Commander::Action::prepareAndApply()
	{
		ensureAttached();
		if (m_applied)
		{
			throw std::logic_error{ "already applied" };
		}
		m_applied = true;
		apply();
	}

	void Commander::Action::prepareAndUnapply()
	{
		ensureAttached();
		if (!m_applied)
		{
			throw std::logic_error{ "not applied" };
		}
		m_applied = false;
		unapply();
	}

	Grid& Commander::Action::grid()
	{
		ensureAttached();
		return m_commander->m_grid;
	}

	const Grid& Commander::Action::grid() const
	{
		ensureAttached();
		return m_commander->m_grid;
	}

	// Commander

	Commander::Commander(Grid& _grid)
		: m_grid{ _grid }
	{}

	Commander::~Commander()
	{
		clear();
	}

	void Commander::apply(Action& _action)
	{
		_action.attach(*this);
		m_unapplied.push_front(&_action);
		redo();
	}

	void Commander::undo()
	{
		if (!canUndo())
		{
			throw std::logic_error{ "cannot undo" };
		}
		Action& action{ *m_applied.front() };
		m_applied.pop_front();
		action.unapply();
		m_unapplied.push_front(&action);
	}

	void Commander::redo()
	{
		if (!canRedo())
		{
			throw std::logic_error{ "cannot redo" };
		}
		Action& action{ *m_unapplied.front() };
		m_unapplied.pop_front();
		action.apply();
		m_applied.push_front(&action);
	}

	void Commander::clear()
	{
		clearUndo();
		clearRedo();
	}

	void Commander::clearUndo()
	{
		limitUndo(0);
	}

	void Commander::clearRedo()
	{
		limitRedo(0);
	}

	void Commander::limitUndo(std::size_t _count)
	{
		while (m_applied.size() > _count)
		{
			Action* action{ m_applied.back() };
			delete action;
			m_applied.pop_back();
		}
	}

	void Commander::limitRedo(std::size_t _count)
	{
		while (m_unapplied.size() > _count)
		{
			Action* action{ m_unapplied.back() };
			delete action;
			m_unapplied.pop_back();
		}
	}

	bool Commander::canUndo() const
	{
		return !m_applied.empty();
	}

	bool Commander::canRedo() const
	{
		return !m_unapplied.empty();
	}

	std::size_t Commander::undoCount() const
	{
		return m_applied.size();
	}

	std::size_t Commander::redoCount() const
	{
		return m_unapplied.size();
	}

	bool Commander::operator==(const Commander& _other) const
	{
		return this == &_other;
	}

}