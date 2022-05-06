#include <HMP/Project.hpp>

namespace HMP
{

	namespace Internal
	{

		ProjectBase::ProjectBase()
			: m_mesher{}, m_commander{ *reinterpret_cast<Project*>(this) }, m_root{}
		{}

		const Commander& ProjectBase::commander() const
		{
			return m_commander;
		}

		Commander& ProjectBase::commander()
		{
			return m_commander;
		}

	}

	const Meshing::Mesher& Project::mesher() const
	{
		return m_mesher;
	}

	const Dag::Element* Project::root() const
	{
		return m_root;
	}

	Dag::Element*& Project::root()
	{
		return m_root;
	}

	Meshing::Mesher& Project::mesher()
	{
		return m_mesher;
	}

}