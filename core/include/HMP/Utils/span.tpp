#ifndef HMP_UTILS_SPAN_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Utils/span.hpp>

namespace HMP::Utils
{

	template<typename TOut, typename TIn>
	std::span<TOut*> span(std::vector<TIn*>& _vector)
	{
		TIn** const begin{ &_vector[0] };
		TIn** const end{ begin + _vector.size() };
		return span<TOut>(begin, end);
	}

	template<typename TOut, typename TIn>
	std::span<TOut*> span(std::span<TIn*>& _span)
	{
		TIn** const begin{ &_span[0] };
		TIn** const end{ begin + _span.size() };
		return span<TOut>(begin, end);
	}

	template<typename TOut, typename TIn>
	std::span<TOut*> span(TIn** _begin, TIn** _end)
	{
		return std::span<TOut*>(reinterpret_cast<TOut**>(_begin), reinterpret_cast<TOut**>(_end));
	}

	template<typename TOut, typename TIn>
	std::span<TOut* const> constSpan(const std::vector<TIn*>& _vector)
	{
		TIn* const* const begin{ &_vector[0] };
		TIn* const* const end{ begin + _vector.size() };
		return constSpan<TOut>(begin, end);
	}

	template<typename TOut, typename TIn>
	std::span<TOut* const> constSpan(const std::span<TIn* const>& _span)
	{
		TIn* const* const begin{ &_span[0] };
		TIn* const* const end{ begin + _span.size() };
		return constSpan<TOut>(begin, end);
	}

	template<typename TOut, typename TIn>
	std::span<TOut* const> constSpan(TIn* const* _begin, TIn* const* _end)
	{
		return std::span<TOut* const>(reinterpret_cast<TOut* const*>(_begin), reinterpret_cast<TOut* const*>(_end));
	}

}