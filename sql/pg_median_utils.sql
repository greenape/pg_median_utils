-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION pg_median_utils" to load this file. \quit
CREATE FUNCTION iterated_median_filter(double precision, int, double precision default 0.0000000001) RETURNS double precision
AS '$libdir/pg_median_utils'
LANGUAGE C IMMUTABLE STRICT WINDOW PARALLEL SAFE;


CREATE FUNCTION median_filter(double precision, int) RETURNS double precision
AS '$libdir/pg_median_utils'
LANGUAGE C IMMUTABLE STRICT WINDOW PARALLEL SAFE;