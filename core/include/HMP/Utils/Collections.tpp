#ifndef HMP_UTILS_COLLECTIONS_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Utils/Collections.hpp>

#include <utility>
#include <stdexcept>

namespace HMP::Utils::Collections
{

	// ZipIterator

	template <typename... TIterators>
	ZipIterator<TIterators...>::ZipIterator(TIterators&&... _iterators)
		: m_iterators{ std::forward<TIterators>(_iterators) ... }
	{}

	template <typename... TIterators>
	ZipIterator<TIterators...>::value_type ZipIterator<TIterators...>::operator*() const
	{
		return std::apply([](auto & ... _iterators) {
			return value_type{ *_iterators... };
			}, m_iterators);
	}

	template <typename... TIterators>
	ZipIterator<TIterators...>& ZipIterator<TIterators...>::operator++()
	{
		std::apply([](auto & ... _iterators) {
			((++_iterators), ...);
			}, m_iterators);
		return *this;
	}

	template <typename... TIterators>
	ZipIterator<TIterators...> ZipIterator<TIterators...>::operator++(int)
	{
		ZipIterator clone{ *this };
		std::apply([](auto & ... _iterators) {
			((_iterators++), ...);
			}, clone.m_iterators);
		return clone;
	}

	template <typename... TIterators>
	ZipIterator<TIterators...>& ZipIterator<TIterators...>::operator+=(int _amount)
	{
		std::apply([](auto & ... _iterators) {
			((_iterators += _amount), ...);
			}, m_iterators);
		return *this;
	}

	template <typename... TIterators>
	ZipIterator<TIterators...>& ZipIterator<TIterators...>::operator--()
	{
		std::apply([](auto & ... _iterators) {
			((--_iterators), ...);
			}, m_iterators);
		return *this;
	}

	template <typename... TIterators>
	ZipIterator<TIterators...> ZipIterator<TIterators...>::operator--(int)
	{
		ZipIterator clone{ *this };
		std::apply([](auto & ... _iterators) {
			((_iterators--), ...);
			}, clone.m_iterators);
		return clone;
	}

	template <typename... TIterators>
	ZipIterator<TIterators...>& ZipIterator<TIterators...>::operator-=(int _amount)
	{
		std::apply([](auto & ... _iterators) {
			((_iterators -= _amount), ...);
			}, m_iterators);
		return *this;
	}

	// ZipIterable

	template <typename... TIterables>
	ZipIterable<TIterables...>::ZipIterable(TIterables&... _iterables)
		: m_iterables{ _iterables... }
	{}

	template <typename... TIterables>
	ZipIterable<TIterables...>::iterator ZipIterable<TIterables...>::begin()
	{
		return std::apply([](auto & ... _iterables) {
			return iterator{ std::begin(_iterables)... };
			}, m_iterables);
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::const_iterator ZipIterable<TIterables...>::begin() const
	{
		return cbegin();
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::const_iterator ZipIterable<TIterables...>::cbegin() const
	{
		return std::apply([](auto & ... _iterables) {
			return iterator{ std::cbegin(_iterables)... };
			}, m_iterables);
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::iterator ZipIterable<TIterables...>::end()
	{
		return std::apply([](auto & ... _iterables) {
			return iterator{ std::end(_iterables)... };
			}, m_iterables);
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::const_iterator ZipIterable<TIterables...>::end() const
	{
		return cend();
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::const_iterator ZipIterable<TIterables...>::cend() const
	{
		return std::apply([](auto & ... _iterables) {
			return iterator{ std::cend(_iterables)... };
			}, m_iterables);
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::const_iterator ZipIterable<TIterables...>::rbegin() const
	{
		return crbegin();
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::const_iterator ZipIterable<TIterables...>::crbegin() const
	{
		return std::apply([](auto & ... _iterables) {
			return iterator{ std::crbegin(_iterables)... };
			}, m_iterables);
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::iterator ZipIterable<TIterables...>::rend()
	{
		return std::apply([](auto & ... _iterables) {
			return iterator{ std::rend(_iterables)... };
			}, m_iterables);
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::const_iterator ZipIterable<TIterables...>::rend() const
	{
		return crend();
	}

	template <typename... TIterables>
	ZipIterable<TIterables...>::const_iterator ZipIterable<TIterables...>::crend() const
	{
		return std::apply([](auto & ... _iterables) {
			return iterator{ std::crend(_iterables)... };
			}, m_iterables);
	}

	// functions

	template <typename... TIterables>
	ZipIterable<TIterables...> zip(TIterables&... _iterables)
	{
		return ZipIterable<TIterables...>{_iterables...};
	}

	template<typename TValue, std::size_t TCount>
	std::vector<TValue> toVector(const std::array<TValue, TCount>& _array)
	{
		return std::vector<TValue>{_array.begin(), _array.end()};
	}

	template<typename TValue, std::size_t TCount>
	std::vector<TValue> toVector(std::array<TValue, TCount>&& _array)
	{
		return std::vector<TValue>{std::make_move_iterator(_array.begin()), std::make_move_iterator(_array.end())};
	}

	template<typename TValue, std::size_t TCount>
	std::array<TValue, TCount > toVector(const std::vector<TValue>& _vector)
	{
		if (_vector.size() != TCount)
		{
			throw std::logic_error{ "size mismatch" };
		}
		return std::array<TValue, TCount>{_vector.begin(), _vector.end()};
	}

	template<typename TValue, std::size_t TCount>
	std::array<TValue, TCount> toVector(std::vector<TValue>&& _vector)
	{
		if (_vector.size() != TCount)
		{
			throw std::logic_error{ "size mismatch" };
		}
		return std::array<TValue, TCount>{std::make_move_iterator(_vector.begin()), std::make_move_iterator(_vector.end())};
	}

}