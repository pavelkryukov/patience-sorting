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

template<typename Deck, typename Compare>
class Installer
{
    using T = typename Deck::value_type;
public:
    static constexpr const bool is_list =
        std::is_same_v<Deck, std::list<T>>;

    explicit Installer(Compare cmp) noexcept
        : cmp(cmp)
    { }

    template<typename It>
    auto install(It begin, It end)
    {
        for (auto it = begin; it != end; ++it)
            get_deck_pointer(*it)->emplace_back(std::move(*it));

        if constexpr (is_list)
            return std::move(decks);
        else
            return install_back(begin);
    }

    auto install(std::list<T>& list)
    {
        static_assert(is_list);
        for (auto it = list.begin(); it != list.end();) {
            auto tmp = it++;
            auto target = get_deck_pointer(*tmp);
            target->splice(target->end(), list, tmp);
        }
        return std::move(decks);
    }

private:
    // Puts partially sorted data back to input
    // Generates ranges of sorted data
    template<typename It>
    auto install_back(It begin) noexcept
    {
        std::deque<std::pair<It, It>> points;
        auto end = begin;
        for (auto& deck : decks) {
            auto range_begin = end;
            end = std::move(deck.begin(), deck.end(), range_begin);
            points.emplace_back(range_begin, end);
        }
        return points;
    }

    auto get_deck_pointer(const T& val)
    {
        auto res = find_deck(val, decks.begin(), decks.end());
        return res != decks.end() ? res : allocate_new_deck();
    }

    auto allocate_new_deck()
    {
        decks.resize(decks.size() + 1);
        return decks.end() - 1;
    }

    template<typename DeckIt>
    auto find_deck(const T& val, DeckIt begin, DeckIt end) const noexcept
    {
        if (end == begin)
            return begin;

        if (end == begin + 1)
            return cmp(begin->back(), val) ? begin : end;

        auto mid = std::next(begin, std::distance(begin, end) / 2);
        return cmp(mid->back(), val)
            ? find_deck(val, begin, mid)
            : find_deck(val, mid, end);
    }

    std::deque<Deck> decks;
    Compare cmp;
};

template<typename Deck, typename It, typename Compare>
auto install(It begin, It end, Compare cmp) noexcept
{
    return Installer<Deck, Compare>(cmp).install(begin, end);
}

template<typename Deck, typename List, typename Compare>
auto install(List& list, Compare cmp) noexcept
{
    return Installer<Deck, Compare>(cmp).install(list);
}

template<typename It, typename Compare>
void merge_range(std::pair<It, It>& r1, std::pair<It, It>& r2, Compare cmp) noexcept
{
    std::inplace_merge(r1.first, r1.second, r2.second, cmp);
    r2.first = r1.first;
}

template<typename T, typename Compare>
void merge_range(std::list<T>& l1, std::list<T>& l2, Compare cmp) noexcept
{
    l2.merge(std::move(l1), cmp);
}

template<typename Compare, typename R>
auto merge(Compare cmp, R&& ranges) noexcept
{
    // Everything is merged.
    if (ranges.size() == 1)
        return ranges.front();

    R new_ranges;

    // Usually last decks are the smallest, so merge them first
    for (int i = ranges.size() - 1; i >= 0; i -= 2) {
        if (i != 0)
            merge_range(ranges[i - 1], ranges[i], cmp);

        new_ranges.emplace_front(std::move(ranges[i]));
    }

    return merge(cmp, std::move(new_ranges));
}

template<typename Deck, typename It, typename Compare>
void sort(It begin, It end, Compare cmp)
{
    auto ranges = install<Deck>(begin, end, cmp);
    auto range = merge(cmp, std::move(ranges));
    if constexpr (std::is_same_v<decltype(range), Deck>)
        std::move(range.begin(), range.end(), begin);
}

template<typename T, typename Compare>
void sort(std::list<T>& list, Compare cmp)
{
    auto ranges = install<std::list<T>>(list, cmp);
    list = merge(cmp, std::move(ranges));
}

} // namespace Patience

template<typename It>
auto patience_sort_cont(It begin, It end)
{
    using T = typename It::value_type;
    Patience::sort<std::deque<T>>(begin, end, Patience::default_compare<T>);
}

template<typename It, typename Compare>
auto patience_sort_cont(It begin, It end, Compare cmp)
{
    using T = typename It::value_type;
    Patience::sort<std::deque<T>>(begin, end, cmp);
}

template<typename It>
auto patience_sort_list(It begin, It end)
{
    using T = typename It::value_type;
    Patience::sort<std::list<T>>(begin, end, Patience::default_compare<T>);
}

template<typename It, typename Compare>
auto patience_sort_list(It begin, It end, Compare cmp)
{
    using T = typename It::value_type;
    Patience::sort<std::list<T>>(begin, end, cmp);
}

template<typename List, typename Compare>
auto patience_sort(List& list, Compare cmp)
{
    Patience::sort(list, cmp);
}

template<typename List>
auto patience_sort(List& list)
{
    using T = typename List::value_type;
    Patience::sort(list, Patience::default_compare<T>);
}
