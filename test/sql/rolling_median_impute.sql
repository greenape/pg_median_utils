SELECT v,  rolling_median_impute(v, 3) over() FROM (VALUES (1), (1.1), (0.9), (NULL), (0.95), (2.1), (1.95), (2.0), (2.05), (3.11), (2.99), (3.05), (3.0)) as t(v);
SELECT v,  rolling_median_impute(v::double precision, 3) over() FROM (VALUES (NULL)) as t(v);
SELECT v,  rolling_median_impute(v, 3) over() FROM (VALUES (1), (1.1), (NULL), (NULL)) as t(v);
SELECT v,  rolling_median_impute(v, 3) over() FROM (VALUES (1), (1.1), (0.9), (NULL), (NULL), (2.1), (NULL), (2.0), (2.05), (3.11), (2.99), (3.05), (NULL)) as t(v);
SELECT v,  rolling_median_impute(v::double precision, 3) over() FROM (VALUES (NULL), (NULL), (NULL)) as t(v);
SELECT v, rolling_median_impute(v, 3) over() FROM (VALUES (9343667.56611), (9357699.85564), (NULL), (9299919.02793), (9343667.56611), (NULL)) as t(v);