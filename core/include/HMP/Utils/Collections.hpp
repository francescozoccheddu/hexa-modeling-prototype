#pragma once

#include <tuple>
#include <iterator>
#include <cstddef>
#include <vector>
#include <array>
#include <type_traits>

namespace HMP::Utils::Collections
{

	template <typename TIterator>
	using IteratorValueType = TIterator::value_type;

	template <typename TIterable>
	using IterableIteratorType = decltype(std::begin(std::declval<TIterable&>()));

	template <typename TIterable>
	using IterableConstIteratorType = decltype(std::cbegin(std::declval<TIterable&>()));

	template <typename... TIterators>
	class ZipIterator final
	{

	private:

		std::tuple<TIterators...> m_iterators;

	public:

		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = std::tuple<IteratorValueType<TIterators>...>;

		ZipIterator(TIterators &&...);

		value_type operator*() const;

		ZipIterator& operator++();
		ZipIterator operator++(int);
		ZipIterator& operator+=(int _amount);
		ZipIterator& operator--();
		ZipIterator operator--(int);
		ZipIterator& operator-=(int _amount);

		friend auto operator<=> (const ZipIterator& _a, const ZipIterator& _b) = default;

	};

	template <typename... TIterables>
	class ZipIterable final
	{

	private:

		std::tuple<TIterables&...> m_iterables;

	public:

		using iterator = ZipIterator<IterableIteratorType<TIterables>...>;
		using const_iterator = ZipIterator<IterableConstIteratorType<TIterables>...>;

		ZipIterable(TIterables&...);

		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;

		iterator rbegin();
		const_iterator rbegin() const;
		const_iterator crbegin() const;
		iterator rend();
		const_iterator rend() const;
		const_iterator crend() const;

	};

	template <typename... TIterables>
	ZipIterable<TIterables...> zip(TIterables&... _iterables);

	template<typename TValue, std::size_t TCount>
	std::vector<TValue> toVector(const std::array<TValue, TCount>& _array);

	template<typename TValue, std::size_t TCount>
	std::vector<TValue> toVector(std::array<TValue, TCount>&& _array);

	template<typename TValue, std::size_t TCount>
	std::array<TValue, TCount> toArray(const std::vector<TValue>& _vector);

	template<typename TValue, std::size_t TCount>
	std::array<TValue, TCount> toArray(std::vector<TValue>&& _vector);

}

#define HMP_UTILS_COLLECTIONS_IMPL
#include <HMP/Utils/Collections.tpp>
#undef HMP_UTILS_COLLECTIONS_IMPL