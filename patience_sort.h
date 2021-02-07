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
#include <vector>

template<typename T>
class ContiniousDeck
{
public:
    using DeckIt = typename std::deque<std::deque<T>>::iterator;

    template<typename RandomIt>
    void produce_output(RandomIt begin) noexcept
    {
        auto mid = begin;
        for (auto& deck : decks)
            mid = merge_deck(deck, begin, mid);
    }

protected:
    std::deque<std::deque<T>> decks;

private:
    template<typename RandomIt>
    auto merge_deck(std::deque<T>& deck, RandomIt begin, RandomIt mid) noexcept
    {
        auto new_mid = std::copy(deck.begin(), deck.end(), mid);
        std::inplace_merge(begin, mid, new_mid);
        return new_mid;
    }
};

template<typename T>
class SparceDeck
{
public:
    using DeckIt = typename std::deque<std::list<T>>::iterator;

    template<typename RandomIt>
    void produce_output(RandomIt begin) noexcept
    {
        std::list<T> result;
        for (auto& deck : decks)
            result.merge(deck);

        std::move(result.begin(), result.end(), begin);
    }

protected:
    std::deque<std::list<T>> decks;
};

template<typename RandomIt, template<typename> class Deck>
class PatienceSort : private Deck<typename RandomIt::value_type>
{
    using T = typename RandomIt::value_type;
    using DeckIt = typename Deck<T>::DeckIt;
public:
    auto sort(RandomIt begin, RandomIt end)
    {
        for (auto it = begin; it != end; ++it)
            fill_to_deck(std::move(*it));

        this->produce_output(begin);
    }

private:
    void fill_to_deck(T val)
    {
        auto res = find_deck(val, this->decks.begin(), this->decks.end());
        if (res == this->decks.end())
            res = allocate_new_deck();

        res->emplace_back(std::move(val));
    }

    auto allocate_new_deck()
    {
        this->decks.resize(this->decks.size() + 1);
        return this->decks.end() - 1;
    }

    auto find_deck(const T& val, DeckIt begin, DeckIt end) const noexcept
    {
        if (end == begin)
            return begin;

        if (end == begin + 1)
            return begin->back() < val ? begin : end;

        auto mid = std::next(begin, std::distance(begin, end) / 2);
        return mid->back() < val ? find_deck(val, begin, mid) : find_deck(val, mid, end);
    }
};

template<typename RandomIt>
auto patience_sort_cont(RandomIt begin, RandomIt end)
{
    PatienceSort<RandomIt, ContiniousDeck>().sort(begin, end);
}

template<typename RandomIt>
auto patience_sort_list(RandomIt begin, RandomIt end)
{
    PatienceSort<RandomIt, SparceDeck>().sort(begin, end);
}
