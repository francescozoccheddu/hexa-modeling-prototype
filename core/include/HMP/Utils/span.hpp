#pragma once
#include <span>
#include <vector>

namespace HMP::Utils
{

	template<typename TOut, typename TIn>
	std::span<TOut*> span(std::vector<TIn*>& _vector);

	template<typename TOut, typename TIn>
	std::span<TOut*> span(std::span<TIn*>& _span);

	template<typename TOut, typename TIn>
	std::span<TOut*> span(TIn** _begin, TIn** _end);

	template<typename TOut, typename TIn>
	std::span<TOut* const> constSpan(const std::vector<TIn*>& _vector);

	template<typename TOut, typename TIn>
	std::span<TOut* const> constSpan(const std::span<TIn* const>& _span);

	template<typename TOut, typename TIn>
	std::span<TOut* const> constSpan(TIn* const* _begin, TIn* const* _end);

}

#define HMP_UTILS_SPAN_IMPL
#include <HMP/Utils/span.tpp>
#undef HMP_UTILS_SPAN_IMPL