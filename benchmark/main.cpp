/*
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

#include "../patience_sort.h"

#include <benchmark/benchmark.h>

#include <list>
#include <random>
#include <stdexcept>

template<typename RandomIt> using SortingFunction = void(*)(RandomIt begin, RandomIt end);
using ListSortingFunction = void(*)(std::list<int>&);

template<class RandomIt>
static void merge_sort(RandomIt first, RandomIt last) noexcept
{
    if (last - first <= 1)
        return;

    auto mid = first + (last - first) / 2;
    merge_sort(first, mid);
    merge_sort(mid, last);
    std::inplace_merge(first, mid, last);
}

template<typename Container>
static auto shuffle(Container* c, int seed)
{
    std::mt19937 gen(seed);
    std::shuffle(c->begin(), c->end(), gen);
}

template<template<typename> typename Сontainer, SortingFunction<typename Сontainer<int>::iterator> func>
static void sorting(benchmark::State& state)
{
    Сontainer<int> data(state.range(0));
    std::iota(data.begin(), data.end(), 0);
    int seed = 0;

    for (auto _ : state) {
        state.PauseTiming();
        shuffle(&data, seed++);
        state.ResumeTiming();
        func(data.begin(), data.end());
    }

    state.SetComplexityN(state.range(0));
}

template<ListSortingFunction func>
static void list_sorting(benchmark::State& state)
{
    std::vector<int> data(state.range(0));
    std::iota(data.begin(), data.end(), 0);
    int seed = 0;

    for (auto _ : state) {
        state.PauseTiming();
        shuffle(&data, seed++);
        std::list<int> list(data.begin(), data.end());
        state.ResumeTiming();
        func(list);
    }

    state.SetComplexityN(state.range(0));
}

static void list_qsort(std::list<int>& list)
{
    list.sort();
}

BENCHMARK_TEMPLATE(sorting, std::vector, std::sort)->RangeMultiplier(2)->Range(1, 1 << 18)->Complexity();
BENCHMARK_TEMPLATE(sorting, std::vector, merge_sort)->RangeMultiplier(2)->Range(1, 1 << 18)->Complexity();
BENCHMARK_TEMPLATE(sorting, std::vector, patience_sort_cont)->RangeMultiplier(2)->Range(1, 1 << 18)->Complexity();
BENCHMARK_TEMPLATE(sorting, std::vector, patience_sort_list)->RangeMultiplier(2)->Range(1, 1 << 18)->Complexity();

BENCHMARK_TEMPLATE(list_sorting, list_qsort)->RangeMultiplier(2)->Range(1, 1 << 18)->Complexity();
BENCHMARK_TEMPLATE(list_sorting, patience_sort<std::list<int>>)->RangeMultiplier(2)->Range(1, 1 << 18)->Complexity();

BENCHMARK_MAIN()
