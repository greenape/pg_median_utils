SELECT v,  iterated_median_filter(v::double precision, 5) over() FROM generate_series(1, 10) as t(v);
SELECT v,  iterated_median_filter(v, 3) over() FROM (VALUES (1), (1.1), (0.9), (1.1), (0.95), (2.1), (1.95), (2.0), (2.05), (3.11), (2.99), (3.05), (3.0)) as t(v);
SELECT v,  iterated_median_filter(v, 3, 'inf'::double precision) over() FROM (VALUES (1), (1.1), (0.9), (1.1), (0.95), (2.1), (1.95), (2.0), (2.05), (3.11), (2.99), (3.05), (3.0)) as t(v);
SELECT v,  iterated_median_filter(v, 3) over() FROM (VALUES (1), (1.1)) as t(v);
SELECT v,  iterated_median_filter(v::double precision, 3) over() FROM (VALUES (NULL), (NULL), (NULL)) as t(v);