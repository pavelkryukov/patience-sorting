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

#include <random>

template<typename RandomIt> using SortingFunction = void(*)(RandomIt begin, RandomIt end);
using VectorSortingFunction = SortingFunction<std::vector<int>::iterator>;

static auto get_random_vector(size_t size)
{
    std::vector<int> data(size);
    std::iota(data.begin(), data.end(), 0);
    std::mt19937 gen(100);
    std::shuffle(data.begin(), data.end(), gen);
    return data;
}

template<VectorSortingFunction func>
static void sorting(benchmark::State& state)
{
    auto data = get_random_vector(state.range(0));

    for (auto _ : state)
        func(data.begin(), data.end());
}

BENCHMARK_TEMPLATE(sorting, std::sort)->RangeMultiplier(2)->Range(1, 1 << 20);
BENCHMARK_TEMPLATE(sorting, patience_sort)->RangeMultiplier(2)->Range(1, 1 << 20);

BENCHMARK_MAIN()
