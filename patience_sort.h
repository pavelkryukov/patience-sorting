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
#include <cmath>
#include <vector>

template<typename RandomIt>
class PatienceSort
{
    using T = typename RandomIt::value_type;
    using DeckIt = typename std::vector<std::vector<T>>::iterator;
public:
    PatienceSort(RandomIt begin, RandomIt end)
        : begin(begin)
        , end(end)
    {
        root = (int)std::sqrt(end - begin);
        decks.reserve(root);
    }

    auto sort()
    {
        fill_decks();
        merge_decks();
    }

private:
    void fill_decks()
    {      
        for (auto it = begin; it != end; ++it)
            fill_to_deck(std::move(*it));
    }

    void fill_to_deck(T val)
    {
        auto res = find_deck(val, decks.begin(), decks.end());
        if (res == decks.end())
            res = allocate_new_deck();

        res->emplace_back(std::move(val));
    }

    auto allocate_new_deck()
    {
        decks.resize(decks.size() + 1);
        decks.back().reserve(root);
        return decks.end() - 1;
    }

    auto find_deck(const T& val, DeckIt begin, DeckIt end) const
    {
        if (end == begin)
            return begin;

        if (end == begin + 1)
            return begin->back() < val ? begin : end;

        auto mid = std::next(begin, std::distance(begin, end) / 2);
        return mid->back() < val ? find_deck(val, begin, mid) : find_deck(val, mid, end);
    }

    void merge_decks()
    {
        auto mid = begin;
        for (auto& deck : decks)
            mid = merge_deck(deck, mid);
    }

    auto merge_deck(std::vector<T>& deck, RandomIt mid)
    {
        auto new_mid = std::copy(deck.begin(), deck.end(), mid);
        std::inplace_merge(begin, mid, new_mid);
        return new_mid;
    }

    RandomIt begin, end;
    size_t root;
    std::vector<std::vector<T>> decks;
};

template<typename RandomIt>
auto patience_sort(RandomIt begin, RandomIt end)
{  
    PatienceSort<RandomIt>(begin, end).sort();
}
