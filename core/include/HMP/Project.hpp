#pragma once

#include <HMP/Commander.hpp>
#include <HMP/Meshing/Mesher.hpp>
#include <HMP/Dag/Element.hpp>
#include <cpputils/mixins/ReferenceClass.hpp>

namespace HMP
{

	class Project;

	namespace Internal
	{

		class ProjectBase : public cpputils::mixins::ReferenceClass
		{

		private:

			friend class Project;

			Dag::Element* m_root;
			Meshing::Mesher m_mesher;
			Commander m_commander;

			ProjectBase();

		public:

			Commander& commander();
			const Commander& commander() const;

		};

	}

	class Project : public Internal::ProjectBase
	{

	private:

		friend class Commander;

		Dag::Element*& root();
		Meshing::Mesher& mesher();

	public:

		Project() = default;
		
		const Dag::Element* root() const;
		const Meshing::Mesher& mesher() const;

	};

}