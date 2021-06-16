# Patience Sorting
C++ implementation of [patience sorting](https://en.wikipedia.org/wiki/Patience_sorting)

### Benchmarking results

Run on (1 X 3200 MHz CPU )
CPU Caches:
  L1 Data 32K (x1)
  L1 Instruction 32K (x1)
  L2 Unified 256K (x1)
  L3 Unified 3072K (x1)
Load Average: 0.33, 0.90, 0.70

| Container | Sorting algorithm | Time, ns   | RMS |
|:---------:|:---------------|:----------:|:---:|
| Vector    | Patience       | 16.5 NlogN | 2%  |
| Vector    | STL quick sort | 3.83 NlogN | 1%  |
| Vector    | STL merge sort | 6.31 NlogN | 3%  |
| List      | Patience       | 51.1 NlogN | 34% |
| List      | STL quick sort | 25.1 NlogN | 49% |

### External bugs reported

 * **[PR clang/49079](https://bugs.llvm.org/show_bug.cgi?id=49079)**: _type mismatch for pointer to template template function_
