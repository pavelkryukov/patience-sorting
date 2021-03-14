/*
 * MIT License
 *
 * Copyright (c) 2021 Pavel I. Kryukov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <algorithm>
#include <deque>
#include <list>

namespace Patience {

template<typename T>
bool default_compare(const T& lhs, const T& rhs) noexcept
{
    return lhs < rhs;
}

template<typename Storage, typename Compare>
class Installer
{
    using T = typename Storage::value_type::value_type;
public:
    Installer(Storage& storage, Compare cmp) : storage(storage), cmp(cmp) { }

    auto get_deck_pointer(const T& val)
    {
        auto res = find_deck(val, storage.begin(), storage.end());
        return res != storage.end() ? res : allocate_new_deck();
    }

private:
    auto allocate_new_deck()
    {
        storage.resize(storage.size() + 1);
        return storage.end() - 1;
    }

    template<typename DeckIt>
    auto find_deck(const T& val, DeckIt begin, DeckIt end) const noexcept
    {
        if (end == begin)
            return begin;

        if (end == begin + 1)
            return cmp(begin->back(), val) ? begin : end;

        auto mid = std::next(begin, std::distance(begin, end) / 2);
        return cmp(mid->back(), val) ? find_deck(val, begin, mid) : find_deck(val, mid, end);
    }

    Storage& storage;
    Compare cmp;
};

template<typename Storage, typename Compare>
auto create_installer(Storage& storage, Compare cmp)
{
    return Installer<Storage, Compare>(storage, cmp);
}

template<typename T>
class Merger
{
public:
    explicit Merger(T* deck) noexcept
        : deck(deck)
    { }

    template<typename R>
    auto merge(R&& ranges) noexcept
    {
        // Everything is merged.
        if (ranges.size() == 1)
            return ranges.front();

        R new_ranges;
        bool left_unmerged = true;

        // Usually last decks are the smallest, so merge them first
        for (int i = ranges.size() - 1; i > 0; i -= 2) {
            auto range = deck->merge_range(ranges[i - 1], ranges[i]);
            new_ranges.emplace_front(std::move(range));
            left_unmerged = i != 1;
        }
        if (left_unmerged)
            new_ranges.emplace_front(std::move(ranges.front()));

        return merge(std::move(new_ranges));      
    }

private:
    T* deck;
};

template<typename Compare, typename RandomIt>
class ContiniousDeck
{
    using Range = std::pair<RandomIt, RandomIt>;
public:
    using T = typename RandomIt::value_type;

    ContiniousDeck(Compare cmp, RandomIt begin)
        : cmp(cmp), begin(begin) { }

    auto get_deck_pointer(const T& val)
    {
        return create_installer(storage, cmp).get_deck_pointer(val);
    }

    void produce_output() noexcept
    {
        move_back();
        Merger(this).merge(std::move(points));
    }

    Range merge_range(const Range& r1, const Range& r2) noexcept
    {
        std::inplace_merge(r1.first, r1.second, r2.second, cmp);
        return Range{r1.first, r2.second};
    }

private:
    auto move_back() noexcept
    {
        auto end = begin;
        for (auto& deck : storage) {
            auto range_begin = end;
            end = std::move(deck.begin(), deck.end(), range_begin);
            points.emplace_back(range_begin, end);
        }
    }

    std::deque<std::deque<T>> storage;

    // Tracks ranges between sorted data
    std::deque<std::pair<RandomIt, RandomIt>> points;

    Compare cmp;
    const RandomIt begin;
};

template<typename T, typename Compare>
class SparceDeck
{
public:
    explicit SparceDeck(Compare cmp) : cmp(cmp) { }

    auto get_deck_pointer(const T& val)
    {
        return create_installer(storage, cmp).get_deck_pointer(val);
    }

    template<typename List>
    void produce_output_list(List& list) noexcept
    {
        list = produce_output_impl();
    }

    auto merge_range(std::list<T>& list1, std::list<T>& list2) noexcept
    {
        list1.merge(list2, cmp);
        return list1;
    }

protected:
    auto produce_output_impl() noexcept
    {
        return Merger(this).merge(std::move(storage));
    }

private:
    std::deque<std::list<T>> storage;

    Compare cmp;
};

template<typename Compare, typename RandomIt>
class SparseDeckRandomOut
    : public SparceDeck<typename RandomIt::value_type, Compare>
{
    using Base = SparceDeck<typename RandomIt::value_type, Compare>;
public:
    SparseDeckRandomOut(Compare cmp, RandomIt begin)
        : Base(cmp), begin(begin) { }

    void produce_output() noexcept
    {
        auto result = this->produce_output_impl();
        std::move(result.begin(), result.end(), begin);
    }

private:
    const RandomIt begin;
};

template<typename RandomIt, template<typename, typename> class Deck, typename Compare>
auto sort_generic(RandomIt begin, RandomIt end, Compare cmp)
{
    Deck<Compare, RandomIt> deck(cmp, begin);
    for (auto it = begin; it != end; ++it)
        deck.get_deck_pointer(*it)->emplace_back(std::move(*it));

    deck.produce_output();
}

} // namespace Patience

template<typename RandomIt>
auto patience_sort_cont(RandomIt begin, RandomIt end)
{
    using T = typename RandomIt::value_type;
    Patience::sort_generic<RandomIt, Patience::ContiniousDeck>(begin, end, Patience::default_compare<T>);
}

template<typename RandomIt, typename Compare>
auto patience_sort_cont(RandomIt begin, RandomIt end, Compare cmp)
{
    Patience::sort_generic<RandomIt, Patience::ContiniousDeck>(begin, end, cmp);
}

template<typename RandomIt>
auto patience_sort_list(RandomIt begin, RandomIt end)
{
    using T = typename RandomIt::value_type;
    Patience::sort_generic<RandomIt, Patience::SparseDeckRandomOut>(begin, end, Patience::default_compare<T>);
}

template<typename RandomIt, typename Compare>
auto patience_sort_list(RandomIt begin, RandomIt end, Compare cmp)
{
    Patience::sort_generic<RandomIt, Patience::SparseDeckRandomOut>(begin, end, cmp);
}

template<typename List, typename Compare>
auto patience_sort(List& list, Compare cmp)
{
    Patience::SparceDeck<typename List::value_type, Compare> deck(cmp);
    for (auto it = list.begin(); it != list.end();) {
        auto tmp = it++;
        auto target = deck.get_deck_pointer(*tmp);
        target->splice(target->end(), list, tmp);
    }

    deck.produce_output_list(list);
}

template<typename List>
auto patience_sort(List& list)
{
    using T = typename List::value_type;
    return patience_sort(list, Patience::default_compare<T>);
}
