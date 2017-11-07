`numeric_quicksort` is a quicksort derivative that uses statistics to sort integer values: when computing the pivot,
it computes the average and standard deviation of the collection, and picks a pair of elements at the positions
corresponding to avg-stdev and avg+stded and partitions the collection into three partitions the following way:

    [ n < avg-stdev | avg-stdev < n < avg+stdev | n > avg+stdev ]

According to Chebyshev's inequality, at least 75% of the collection should be in the middle partitions, while the
critically low & high values are not in it. The next step is to compute the average of the middle partition, and
use the resulting value as a pivot to partition it again. Since the lowest and highest values have been excluded
during the first partitioning step, the average of the values in the middle partition should yield a good enough
pivot for a quicksort, with high probabilities of making the algorithm O(n log n), even though I'm not dedicated
enough to prove it. The whole collection is then partitioned roughly as follows: 

   [ n < avg-stdev | avg-stdev < n < pivot | pivot < n < avg+stdev | n > avg+stdev ]

The next step is to recursively sort the different partitions, while keeping the old quicksort trick of recursing into
the smallest partitions first in order to limit potential stack recursion to O(log n). We first sort the first and
last partitions since they're both guaranteed to contain no more than 12.5% of the original collection once again
thanks to Chebyshev's inequality. Then we compare the sizes of the two middle partitions to decide which to sort first.
When a collection is small enough, insertion sort is called and stops the recursion.

This algorithm was only designed for fun and isn't anywhere near fast compared to half-decent sorting algorithms. It's
just that sorting with a bit of statistics seemed quite fun and I was glad I could come up with one that used a few
tools from statistics with a guarantee to (almost) always find a pivot that doesn't trigger one of quicksort's worst
cases. The C++ function works with bidirectional iterators and can be called as follows:

    numeric_quicksort(std::begin(collection), std::end(collection));
