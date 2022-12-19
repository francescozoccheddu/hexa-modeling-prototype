#pragma once 

#include <HMP/Utils/MapRanged.hpp>

namespace HMP::Utils
{

    namespace Internal
    {

        template<typename TIterable>
        using NonConstDerefRange = decltype(cpputils::range::of(std::declval<TIterable&>()).dereference());

        template<typename TIterable>
        using ConstDerefRange = decltype(cpputils::range::ofc(std::declval<const TIterable&>()).dereference().immutable());

        template<typename TIterable>
        constexpr NonConstDerefRange<TIterable> makeNonConstDerefRange(TIterable& _iterable)
        {
            return cpputils::range::of(_iterable).dereference();
        }

        template<typename TIterable>
        constexpr ConstDerefRange<TIterable> makeConstDerefRange(const TIterable& _iterable)
        {
            return cpputils::range::ofc(_iterable).dereference().immutable();
        }

        template<typename TIterable>
        using NonConstDerefRangeIterator = typename NonConstDerefRange<TIterable>::Iterator;

        template<typename TIterable>
        using ConstDerefRangeIterator = typename ConstDerefRange<TIterable>::Iterator;

    }

    template<typename TIterable>
    class DerefRanged: public cpputils::range::ConstAndNonConstRanged<Internal::ConstDerefRangeIterator<TIterable>, Internal::NonConstDerefRangeIterator<TIterable>, cpputils::range::RangeMaker<TIterable>::compTimeSize>
    {

    private:

        TIterable& m_iterable;

        virtual Internal::NonConstDerefRange<TIterable> range() override final
        {
            return Internal::makeNonConstDerefRange<TIterable>(m_iterable);
        }

        virtual Internal::ConstDerefRange<TIterable> range() const override final
        {
            return Internal::makeConstDerefRange<TIterable>(m_iterable);
        }

    protected:

        DerefRanged(TIterable& _iterable): m_iterable{ _iterable } {}

    };

    template<typename TIterable>
    class ConstDerefRanged: public cpputils::range::ConstRanged<Internal::ConstDerefRangeIterator<TIterable>, cpputils::range::RangeMaker<TIterable>::compTimeSize>
    {

    private:

        const TIterable& m_iterable;

        virtual Internal::ConstDerefRange<TIterable> range() const override final
        {
            return Internal::makeConstDerefRange<TIterable>(m_iterable);
        }

    protected:

        ConstDerefRanged(const TIterable& _iterable): m_iterable{ _iterable } {}

    };

}