#ifndef HMP_UTILS_SETVIEW_IMPL
#error __FILE__ should not be directly included
#endif

#include <HMP/Utils/SetView.hpp>
#include <stdexcept>

namespace HMP::Utils
{

	// SetViewIterator

	template<typename TInternal, typename TView>
	SetViewIterator<TInternal, TView>::SetViewIterator(std::unordered_set<TInternal*>::iterator _iterator)
		: m_iterator{ _iterator }
	{}

	template<typename TInternal, typename TView>
	typename SetViewIterator<TInternal, TView>::reference SetViewIterator<TInternal, TView>::operator*() const
	{
		return *reinterpret_cast<TView*>(*m_iterator);
	}

	template<typename TInternal, typename TView>
	typename SetViewIterator<TInternal, TView>::pointer SetViewIterator<TInternal, TView>::operator->() const
	{
		return reinterpret_cast<TView*>(*m_iterator);
	}

	template<typename TInternal, typename TView>
	SetViewIterator<TInternal, TView>& SetViewIterator<TInternal, TView>::operator++()
	{
		++m_iterator;
		return *this;
	}

	template<typename TInternal, typename TView>
	SetViewIterator<TInternal, TView> SetViewIterator<TInternal, TView>::operator++(int)
	{
		SetViewIterator clone{ *this };
		clone.m_iterator++;
		return clone;
	}

	template<typename TInternal, typename TView>
	SetViewIterator<TInternal, TView>& SetViewIterator<TInternal, TView>::operator+=(int _amount)
	{
		m_iterator += _amount;
		return *this;
	}

	template<typename TInternal, typename TView>
	SetViewIterator<TInternal, TView>& SetViewIterator<TInternal, TView>::operator--()
	{
		--m_iterator;
		return *this;
	}

	template<typename TInternal, typename TView>
	SetViewIterator<TInternal, TView> SetViewIterator<TInternal, TView>::operator--(int)
	{
		SetViewIterator clone{ *this };
		clone.m_iterator--;
		return clone;
	}

	template<typename TInternal, typename TView>
	SetViewIterator<TInternal, TView>& SetViewIterator<TInternal, TView>::operator-=(int _amount)
	{
		m_iterator -= _amount;
		return *this;
	}

	// SetView

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::SetView(std::shared_ptr<std::unordered_set<TInternal*>> _data)
		: m_data{ _data }
	{}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::SetView()
		: m_data{ new std::unordered_set<TInternal*>{} }
	{}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::SetView(std::size_t _capacity) : SetView{}
	{
		m_data->reserve(_capacity);
	}

	template<typename TInternal, typename TView>
	template<typename TNewView>
	SetView<TInternal, TNewView> SetView<TInternal, TView>::view()
	{
		return SetView<TInternal, TNewView>{m_data};
	}

	template<typename TInternal, typename TView>
	bool SetView<TInternal, TView>::add(TView& _item)
	{
		return m_data->insert(reinterpret_cast<TInternal*>(&_item)).second;
	}

	template<typename TInternal, typename TView>
	void SetView<TInternal, TView>::addOrThrow(TView& _item)
	{
		if (!add(_item))
		{
			throw std::logic_error{ "not added" };
		}
	}

	template<typename TInternal, typename TView>
	bool SetView<TInternal, TView>::remove(const TView& _item)
	{
		return m_data->erase(const_cast<TInternal*>(reinterpret_cast<const TInternal*>(&_item)));
	}

	template<typename TInternal, typename TView>
	void SetView<TInternal, TView>::removeOrThrow(const TView& _item)
	{
		if (!remove(_item))
		{
			throw std::logic_error{ "not removed" };
		}
	}

	template<typename TInternal, typename TView>
	bool SetView<TInternal, TView>::has(const TView& _item) const
	{
		return m_data->contains(reinterpret_cast<const TInternal*>(&_item));
	}

	template<typename TInternal, typename TView>
	bool SetView<TInternal, TView>::any(std::function<bool(const TView&)> _predicate) const
	{
		for (const TView& item : *this)
		{
			if (_predicate(item))
			{
				return true;
			}
		}
		return false;
	}

	template<typename TInternal, typename TView>
	bool SetView<TInternal, TView>::all(std::function<bool(const TView&)> _predicate) const
	{
		for (const TView& item : *this)
		{
			if (!_predicate(item))
			{
				return false;
			}
		}
		return true;
	}

	template<typename TInternal, typename TView>
	std::size_t SetView<TInternal, TView>::size() const
	{
		return m_data->size();
	}

	template<typename TInternal, typename TView>
	void SetView<TInternal, TView>::clear()
	{
		m_data->clear();
	}

	template<typename TInternal, typename TView>
	bool SetView<TInternal, TView>::empty() const
	{
		return m_data->empty();
	}

	template<typename TInternal, typename TView>
	TView& SetView<TInternal, TView>::single()
	{
		if (size() != 1)
		{
			throw std::logic_error{ "not a singleton" };
		}
		return *begin();
	}

	template<typename TInternal, typename TView>
	const TView& SetView<TInternal, TView>::single() const
	{
		return const_cast<SetView*>(this)->single();
	}

	template<typename TInternal, typename TView>
	TView& SetView<TInternal, TView>::first()
	{
		if (empty())
		{
			throw std::logic_error{ "empty" };
		}
		return *begin();
	}

	template<typename TInternal, typename TView>
	const TView& SetView<TInternal, TView>::first() const
	{
		return const_cast<SetView*>(this)->first();
	}

	template<typename TInternal, typename TView>
	TView& SetView<TInternal, TView>::last()
	{
		if (empty())
		{
			throw std::logic_error{ "empty" };
		}
		return *rbegin();
	}

	template<typename TInternal, typename TView>
	const TView& SetView<TInternal, TView>::last() const
	{
		return const_cast<SetView*>(this)->last();
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::Iterator SetView<TInternal, TView>::begin()
	{
		return Iterator{ m_data->begin() };
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::Iterator SetView<TInternal, TView>::rbegin()
	{
		return Iterator{ m_data->rbegin() };
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::Iterator SetView<TInternal, TView>::end()
	{
		return Iterator{ m_data->end() };
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::Iterator SetView<TInternal, TView>::rend()
	{
		return Iterator{ m_data->rend() };
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::ConstIterator SetView<TInternal, TView>::begin() const
	{
		return cbegin();
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::ConstIterator SetView<TInternal, TView>::rbegin() const
	{
		return crbegin();
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::ConstIterator SetView<TInternal, TView>::end() const
	{
		return cend();
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::ConstIterator SetView<TInternal, TView>::rend() const
	{
		return crend();
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::ConstIterator SetView<TInternal, TView>::cbegin() const
	{
		return ConstIterator{ m_data->cbegin() };
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::ConstIterator SetView<TInternal, TView>::crbegin() const
	{
		return ConstIterator{ m_data->crbegin() };
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::ConstIterator SetView<TInternal, TView>::cend() const
	{
		return ConstIterator{ m_data->cend() };
	}

	template<typename TInternal, typename TView>
	SetView<TInternal, TView>::ConstIterator SetView<TInternal, TView>::crend() const
	{
		return ConstIterator{ m_data->crend() };
	}

	template<typename TInternal, typename TView>
	std::unordered_set<TView*> SetView<TInternal, TView>::set()
	{
		std::unordered_set<TView*> set{};
		set.reserve(size());
		for (TView& item : *this)
		{
			set.insert(&item);
		}
		return set;
	}

	template<typename TInternal, typename TView>
	std::unordered_set<const TView*> SetView<TInternal, TView>::constSet() const
	{
		std::unordered_set<const TView*> set{};
		set.reserve(size());
		for (const TView& item : *this)
		{
			set.insert(&item);
		}
		return set;
	}

	template<typename TInternal, typename TView>
	std::vector<TView*> SetView<TInternal, TView>::vector()
	{
		std::vector<TView*> vector{};
		vector.reserve(size());
		for (TView& item : *this)
		{
			vector.push_back(&item);
		}
		return vector;
	}

	template<typename TInternal, typename TView>
	std::vector<const TView*> SetView<TInternal, TView>::constVector() const
	{
		std::vector<const TView*> vector{};
		vector.reserve(size());
		for (const TView& item : *this)
		{
			vector.push_back(&item);
		}
		return vector;
	}

}