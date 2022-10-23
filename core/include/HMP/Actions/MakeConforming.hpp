#pragma once

#include <HMP/Meshing/types.hpp>
#include <HMP/Dag/NodeHandle.hpp>
#include <HMP/Dag/Refine.hpp>
#include <HMP/Commander.hpp>
#include <vector>
#include <utility>

namespace HMP::Actions
{

	class MakeConforming final : public Commander::Action
	{

	private:

		static constexpr bool c_requireNoMove{ false };
		static constexpr bool c_requireNoDelete{ false };
		std::vector<std::pair<Dag::NodeHandle<Dag::Refine>, Dag::Element*>> m_operations;
		bool m_prepared;

		void apply() override;
		void unapply() override;

		using RefinementMap = std::unordered_map<Meshing::ERefinementScheme, std::vector<Dag::Refine*>>;

		RefinementMap findStandardRefinements();

		std::vector<Dag::Refine*> installInset(const std::vector<Dag::Refine*>& _insets);
		std::vector<Dag::Refine*> installAdapterEdgeSubdivide3x3(const std::vector<Dag::Refine*>& _sub3x3s);
		std::vector<Dag::Refine*> installAdapterFaceSubdivide3x3(const std::vector<Dag::Refine*>& _sub3x3s);
		std::vector<Dag::Refine*> installSubdivide3x3(const std::vector<Dag::Refine*>& _sub3x3s);
		std::vector<Dag::Refine*> installAdapter2FacesSubdivide3x3(const std::vector<Dag::Refine*>& _sub3x3s);

	public:

		MakeConforming();

		std::vector<std::pair<const Dag::Refine*, const Dag::Element*>> operations() const;

	};

}
