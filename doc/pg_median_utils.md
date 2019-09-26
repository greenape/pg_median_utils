pg_median_utils 0.0.6
=====================


An extension for PostgreSQL >= 9.6 containing some median-related utilities.

At this time, provides five window functions:
 - `median_filter` which behaves the same as SciPy's [medfilt](https://docs.scipy.org/doc/scipy-0.14.0/reference/generated/scipy.signal.medfilt.html)
 - `iterated_median_filter`, which applies the median filter iteratively until it converges (no change greater than some small value).
 - `rolling_median`, which calculates the median over the preceding `n` rows and returns `NULL` for the first `n` rows
 - `backfilled_rolling_median`, which calculates the median over the preceding `n` rows and backfills the first `n` rows with the median over them (returns `NULL` if there are less rows than the window size).
 - `rolling_median_impute`:
    1. Calculates the rolling median over all _non-null_ rows with window size `n`
    1. Backfills the first `n` rows with the first median over a complete window
    1. _Forward_-fills any null row with the nearest rolling median value

Usage
-----

### Median filters

Use with any double precision column, for example:


    SELECT v,  median_filter(v::double precision, 5) over() FROM generate_series(1, 10) as t(v);
     v  | median_filter 
    ----+---------------
      1 |             1
      2 |             2
      3 |             3
      4 |             4
      5 |             5
      6 |             6
      7 |             7
      8 |             8
      9 |             8
     10 |             8
    (10 rows)


Or for an iterated version:


    SELECT v,  iterated_median_filter(v, 3) over() FROM (VALUES (1), (1.1), (0.9), (1.1), (0.95), (2.1), (1.95), (2.0), (2.05), (3.11), (2.99), (3.05), (3.0)) as t(v);
      v   | iterated_median_filter 
    ------+------------------------
        1 |                      1
      1.1 |                      1
      0.9 |                      1
      1.1 |                    1.1
     0.95 |                    1.1
      2.1 |                   1.95
     1.95 |                      2
      2.0 |                      2
     2.05 |                   2.05
     3.11 |                   2.99
     2.99 |                      3
     3.05 |                      3
      3.0 |                      3
    (13 rows)


Comparing the two:


    SELECT median_filter(v, 3) over(), iterated_median_filter(v, 3, 0.0000001) over() FROM (VALUES (1), (1.1), (0.9), (1.1), (0.95), (2.1), (1.95), (2.0), (2.05), (3.11), (2.99), (3.05), (3.0)) as t(v);
    
    median_filter | iterated_median_filter 
    ---------------+------------------------
                 1 |                      1
                 1 |                      1
               1.1 |                      1
              0.95 |                    1.1
               1.1 |                    1.1
              1.95 |                   1.95
                 2 |                      2
                 2 |                      2
              2.05 |                   2.05
              2.99 |                   2.99
              3.05 |                      3
                 3 |                      3
                 3 |                      3
    (13 rows)

    
### Rolling medians

Usage of the rolling median functions is similar to the filters:


    SELECT v,  rolling_median(v::double precision, 5) over() FROM generate_series(1, 10) as t(v);
     v  | rolling_median 
    ----+----------------
      1 |               
      2 |               
      3 |               
      4 |               
      5 |              3
      6 |              4
      7 |              5
      8 |              6
      9 |              7
     10 |              8
    (10 rows)


Or for the backfilled equivalent:


    SELECT v,  backfilled_rolling_median(v::double precision, 5) over() FROM generate_series(1, 10) as t(v);
     v  | backfilled_rolling_median 
    ----+---------------------------
      1 |                         3
      2 |                         3
      3 |                         3
      4 |                         3
      5 |                         3
      6 |                         4
      7 |                         5
      8 |                         6
      9 |                         7
     10 |                         8
    (10 rows)


Imputation
----------


    SELECT v, rolling_median_impute(v, 3) over() FROM (VALUES (1), (1.1), (0.9), (NULL), (NULL), (2.1), (NULL), (2.0), (2.05), (3.11), (2.99), (3.05), (NULL)) as t(v);
     rolling_median_impute 
    -----------------------
                         1
                       1.1
                       0.9
                       1.1
                       1.1
                       2.1
                         2
                         2
                      2.05
                      3.11
                      2.99
                      3.05
                      2.99
    (13 rows)


Support
-------

This library is stored in an open
[GitHub repository](https://github.com/theory/pg-semver). Feel free to fork
and contribute! Please file bug reports via
[GitHub Issues](https://github.com/theory/pg-semver/issues/).

Authors
-------

* [Jono Gray](https://github.com/greenape)

Copyright and License
---------------------

MIT License

Copyright (c) 2019 Jono Gray

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
