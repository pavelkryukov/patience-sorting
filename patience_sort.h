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

template<typename T>
class ContiniousDeck
{
public:
    template<typename RandomIt>
    void produce_output(RandomIt begin) noexcept
    {
        auto mid = begin;
        for (auto& deck : storage)
            mid = merge(deck, begin, mid);
    }

protected:
    std::deque<std::deque<T>> storage;

private:
    template<typename RandomIt>
    static auto merge(std::deque<T>& deck, RandomIt begin, RandomIt mid) noexcept
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
    template<typename RandomIt>
    void produce_output(RandomIt begin) noexcept
    {
        auto result = produce_output();
        std::move(result.begin(), result.end(), begin);
    }

    template<typename List>
    void produce_output_list(List& list) noexcept
    {
        list.splice(list.begin(), produce_output());
    }

protected:
    std::deque<std::list<T>> storage;

private:
    auto produce_output() noexcept
    {
        std::list<T> result;
        for (auto& deck : storage)
            result.merge(deck);
        
        return result;
    }
};

template<typename T, template<typename> class Deck>
class DeckFinder : public Deck<T>
{
public:
    auto get_deck_pointer(const T& val)
    {
        auto res = find_deck(val, this->storage.begin(), this->storage.end());
        return res != this->storage.end() ? res : allocate_new_deck();
    }

private:
    auto allocate_new_deck()
    {
        this->storage.resize(this->storage.size() + 1);
        return this->storage.end() - 1;
    }

    template<typename DeckIt>
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

template<typename RandomIt, template<typename T> class Deck>
auto patience_sort_generic(RandomIt begin, RandomIt end)
{
    DeckFinder<typename RandomIt::value_type, Deck> deck;
    for (auto it = begin; it != end; ++it)
        deck.get_deck_pointer(*it)->emplace_back(std::move(*it));

    deck.produce_output(begin);
}

template<typename RandomIt>
auto patience_sort_cont(RandomIt begin, RandomIt end)
{
    patience_sort_generic<RandomIt, ContiniousDeck>(begin, end);
}

template<typename RandomIt>
auto patience_sort_list(RandomIt begin, RandomIt end)
{
    patience_sort_generic<RandomIt, SparceDeck>(begin, end);
}

template<typename List>
auto patience_sort(List& list)
{
    DeckFinder<typename List::value_type, SparceDeck> deck;
    for (auto it = list.begin(); it != list.end();) {
        auto tmp = it++;
        auto target = deck.get_deck_pointer(*tmp);
        target->splice(target->end(), list, tmp);
    }

    deck.produce_output_list(list);
}
