#include <HMP/Commander.hpp>

#include <HMP/Project.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <stdexcept>

namespace HMP
{

	// Commander::ActionBase

	Commander::ActionBase::ActionBase()
		: m_commander{}, m_applied{ false }
	{}

	void Commander::ActionBase::ensureAttached() const
	{
		if (!attached())
		{
			throw std::logic_error{ "not attached" };
		}
	}

	void Commander::ActionBase::attach(Commander& _commander)
	{
		if (attached())
		{
			throw std::logic_error{ "already attached" };
		}
		m_commander = &_commander;
	}

	void Commander::ActionBase::prepareAndApply()
	{
		ensureAttached();
		if (m_applied)
		{
			throw std::logic_error{ "already applied" };
		}
		m_applied = true;
		apply();
	}

	void Commander::ActionBase::prepareAndUnapply()
	{
		ensureAttached();
		if (!m_applied)
		{
			throw std::logic_error{ "not applied" };
		}
		m_applied = false;
		unapply();
	}

	Meshing::Mesher& Commander::ActionBase::mesher()
	{
		ensureAttached();
		return m_commander->m_project.mesher();
	}

	const Meshing::Mesher& Commander::ActionBase::mesher() const
	{
		ensureAttached();
		return m_commander->m_project.mesher();
	}

	Dag::Element*& Commander::ActionBase::root()
	{
		ensureAttached();
		return m_commander->m_project.root();
	}

	const Dag::Element* Commander::ActionBase::root() const
	{
		ensureAttached();
		return m_commander->m_project.root();
	}

	bool Commander::ActionBase::attached() const
	{
		return m_commander;
	}

	bool Commander::ActionBase::applied() const
	{
		return m_applied;
	}

	// Commander::StackBase

	Commander::StackBase::StackBase()
		: m_data{}, m_limit{ 1000 }, cpputils::collections::DereferenceIterable<std::deque<Action*>, const Action&, const Action&>{ m_data }
	{}

	Commander::Action& Commander::StackBase::pop()
	{
		if (empty())
		{
			throw std::logic_error{ "empty" };
		}
		Action& action{ *m_data.front() };
		m_data.pop_front();
		return action;
	}

	void Commander::StackBase::push(Action& _action)
	{
		m_data.push_front(&_action);
		keepLatest(m_limit);
	}

	std::size_t Commander::StackBase::limit() const
	{
		return m_limit;
	}

	void Commander::StackBase::limit(std::size_t _count)
	{
		m_limit = _count;
		keepLatest(m_limit);
	}

	void Commander::StackBase::removeOldest(std::size_t _count)
	{
		while (!m_data.empty() && _count > 0)
		{
			Action* action{ m_data.back() };
			delete action;
			m_data.pop_back();
			_count--;
		}
	}

	void Commander::StackBase::keepLatest(std::size_t _count)
	{
		while (m_data.size() > _count)
		{
			Action* action{ m_data.back() };
			delete action;
			m_data.pop_back();
		}
	}

	void Commander::StackBase::clear()
	{
		auto it{ m_data.rbegin() };
		const auto end{ m_data.rend() };
		while (it != end)
		{
			delete *it;
			it++;
		}
		m_data.clear();
	}

	// Commander

	Commander::Commander(Project& _project)
		: m_project{ _project }, m_applied{}, m_unapplied{}
	{}

	Commander::~Commander()
	{
		m_unapplied.clear();
		m_applied.clear();
	}

	void Commander::apply(Action& _action)
	{
		_action.attach(*this);
		m_unapplied.clear();
		m_unapplied.push(_action);
		redo();
	}

	void Commander::undo()
	{
		if (!canUndo())
		{
			throw std::logic_error{ "cannot undo" };
		}
		Action& action{ m_applied.pop() };
		action.unapply();
		m_unapplied.push(action);
	}

	void Commander::redo()
	{
		if (!canRedo())
		{
			throw std::logic_error{ "cannot redo" };
		}
		Action& action{ m_unapplied.pop() };
		action.apply();
		m_applied.push(action);
	}

	bool Commander::canUndo() const
	{
		return !m_applied.empty();
	}

	bool Commander::canRedo() const
	{
		return !m_unapplied.empty();
	}


	Commander::Stack& Commander::unapplied()
	{
		return m_unapplied;
	}

	const Commander::Stack& Commander::unapplied() const
	{
		return m_unapplied;
	}

	Commander::Stack& Commander::applied()
	{
		return m_applied;
	}

	const Commander::Stack& Commander::applied() const
	{
		return m_applied;
	}

}