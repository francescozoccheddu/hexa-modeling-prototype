#include <HMP/Project.hpp>

namespace HMP
{

	namespace Internal
	{

		ProjectBase::ProjectBase()
			: m_grid{}, m_commander{ *reinterpret_cast<Project*>(this) }, m_root{}
		{}

		const Commander& ProjectBase::commander() const
		{
			return m_commander;
		}

		Commander& ProjectBase::commander()
		{
			return m_commander;
		}

		const Grid& ProjectBase::grid() const
		{
			return m_grid;
		}

		const Dag::Element* ProjectBase::root() const
		{
			return m_root;
		}

	}

	Dag::Element*& Project::root()
	{
		return m_root;
	}

	Grid& Project::grid()
	{
		return m_grid;
	}

}