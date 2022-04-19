#pragma once

#include <iterator>
#include <cstddef>
#include <compare>
#include <unordered_set>
#include <concepts>
#include <memory>

namespace HMP::Utils
{

	template<typename TInternal, typename TView>
	class SetViewIterator final
	{

	private:

		template<typename TSetViewInternal, typename TSetViewView>
		friend class SetView;

		std::unordered_set<TInternal*>::iterator m_iterator;

		SetViewIterator(std::unordered_set<TInternal*>::iterator _iterator);

	public:

		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = TView&;
		using pointer = TView*;
		using reference = TView&;

		reference operator*() const;
		pointer operator->() const;

		SetViewIterator& operator++();
		SetViewIterator operator++(int);
		SetViewIterator& operator+=(int _amount);
		SetViewIterator& operator--();
		SetViewIterator operator--(int);
		SetViewIterator& operator-=(int _amount);

		friend auto operator<=> (const SetViewIterator& _a, const SetViewIterator& _b) = default;

	};

	template<typename TInternal, typename TView = TInternal>
	class SetView final
	{

	private:

		template<typename TSetViewInternal, typename TSetViewView>
		friend class SetView;

		std::shared_ptr<std::unordered_set<TInternal*>> m_data;

		SetView(std::shared_ptr<std::unordered_set<TInternal*>> _data);

	public:

		using Iterator = SetViewIterator<TInternal, TView>;
		using ConstIterator = SetViewIterator<TInternal, const TView>;

		SetView();

		template <typename TNewView>
		SetView<TInternal, TNewView> view();

		bool add(TView& _item);
		bool remove(const TView& _item);
		bool has(const TView& _item) const;
		std::size_t size() const;
		void clear();
		bool empty() const;

		Iterator begin();
		Iterator rbegin();
		Iterator end();
		Iterator rend();
		ConstIterator begin() const;
		ConstIterator rbegin() const;
		ConstIterator end() const;
		ConstIterator rend() const;
		ConstIterator cbegin() const;
		ConstIterator crbegin() const;
		ConstIterator cend() const;
		ConstIterator crend() const;

	};

}


#define HMP_UTILS_SETVIEW_IMPL
#include <HMP/Utils/SetView.tpp>
#undef HMP_UTILS_SETVIEW_IMPL