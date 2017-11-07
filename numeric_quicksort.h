/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Morwenn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef NUMERIC_QUICK_SORT_H_
#define NUMERIC_QUICK_SORT_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iterator>
#include <utility>

namespace detail
{
    template<typename Iterator, typename Size>
    auto average(Iterator first, Iterator last, Size size)
        -> long double
    {
        if (size == 0) return 0.0L;

        std::intmax_t accumulator = 0;
        long double res = 0.0L;

        while (first != last) {
            std::intmax_t tmp;
            if (__builtin_add_overflow(*first, accumulator, &tmp)) {
                res += static_cast<long double>(accumulator) / static_cast<long double>(size);
                accumulator = *first;
            } else {
                accumulator += *first;
            }
            ++first;
        }

        res += static_cast<long double>(accumulator) / static_cast<long double>(size);
        return res;
    }

    template<typename Iterator, typename Size>
    auto numeric_pivots(Iterator first, Iterator last, Size size)
        -> std::pair<long double, long double>
    {
        // Compute average
        auto avg = average(first, last, size);

        // Compute standard deviation
        long double std_dev = 0.0L;
        for (auto it = first ; it != last ; ++it) {
            std_dev += (*it - avg) * (*it - avg);
        }
        std_dev = std::sqrt(std_dev) / std::sqrt(static_cast<long double>(size));

        // Should keep 75% of values according to Chebyshev's inequality
        return std::make_pair(avg - std_dev, avg + std_dev);
    }

    template<typename BidirectionalIterator, typename Compare>
    auto insertion_sort(BidirectionalIterator first, BidirectionalIterator last, Compare compare)
        -> void
    {
        if (first == last) return;

        for (BidirectionalIterator cur = std::next(first) ; cur != last ; ++cur) {
            BidirectionalIterator sift = cur;
            BidirectionalIterator sift_1 = std::prev(cur);

            // Compare first so we can avoid 2 moves for
            // an element already positioned correctly.
            if (compare(*sift, *sift_1)) {
                auto tmp = std::move(*sift);
                do {
                    *sift = std::move(*sift_1);
                } while (--sift != first && compare(tmp, *--sift_1));
                *sift = std::move(tmp);
            }
        }
    }

    template<typename BidirectionalIterator, typename Size, typename Compare>
    auto numeric_quicksort_impl(BidirectionalIterator first, BidirectionalIterator last,
                                Size size, Compare compare)
        -> void
    {
        // If the collection is small enough, fall back to
        // another sorting algorithm
        if (size < 32) {
            insertion_sort(std::move(first), std::move(last), std::move(compare));
            return;
        }

        auto pivots = numeric_pivots(first, last, size);
        BidirectionalIterator middle1 = std::partition(
            first, last,
            [&](const auto& elem) { return compare(elem, pivots.first); }
        );
        BidirectionalIterator middle2 = std::partition(
            middle1, last,
            [&](const auto& elem) { return not compare(pivots.second, elem); }
        );

        // More accurate pivot since we removed the extremes
        auto new_pivot = average(middle1, middle2, std::distance(middle1, middle2));
        BidirectionalIterator middle3 = std::partition(
            middle1, middle2,
            [&](const auto& elem) { return compare(elem, new_pivot); }
        );
        BidirectionalIterator middle4 = std::partition(
            middle3, middle2,
            [&](const auto& elem) { return not compare(new_pivot, elem); }
        );

        // Recursive call: Chebyshev's inequality ensures that at least
        // 75% of the data will be in the middle partition, so computing
        // the size of the middle one with subtractions should be cheaper
        typename std::iterator_traits<BidirectionalIterator>::difference_type sizes[] = {
            std::distance(first, middle1),
            std::distance(middle1, middle3),
            std::distance(middle4, middle2),
            std::distance(middle2, last)
        };

        // Recurse in the smallest partitions first
        numeric_quicksort_impl(first, middle1, sizes[0], compare);
        numeric_quicksort_impl(middle2, last, sizes[3], compare);

        // Recurse in the smallest of the middle partitions first
        if (sizes[1] < sizes[2]) {
            numeric_quicksort_impl(middle1, middle3, sizes[1], compare);
            numeric_quicksort_impl(middle4, middle2, sizes[2], compare);
        } else {
            numeric_quicksort_impl(middle4, middle2, sizes[2], compare);
            numeric_quicksort_impl(middle1, middle3, sizes[1], compare);
        }
    }
}

template<typename BidirectionalIterator, typename Compare=std::less<>>
auto numeric_quicksort(BidirectionalIterator first, BidirectionalIterator last,
                       Compare compare={})
    -> void
{
    detail::numeric_quicksort_impl(std::move(first), std::move(last),
                                   std::distance(first, last),
                                   std::move(compare));
}

#endif // NUMERIC_QUICK_SORT_H_
