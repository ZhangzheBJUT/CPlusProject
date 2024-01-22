/* funnel_analysis */
CREATE OR REPLACE FUNCTION funnel_detail_append( p_state internal, total_step integer, window_size bigint, event_time bigint, step integer)
  RETURNS internal
  AS 'MODULE_PATHNAME','funnel_detail_append'
  LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION funnel_detail_combine( p_state_1 internal, p_state_2 internal)
  RETURNS internal
  AS 'MODULE_PATHNAME','funnel_detail_combine'
  LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION funnel_detail_reducer(internal)
  RETURNS integer
  AS 'MODULE_PATHNAME','funnel_detail_reducer'
  LANGUAGE C IMMUTABLE;

/* serialize data */
CREATE OR REPLACE FUNCTION funnel_detail_serialize(p_pointer internal)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'funnel_detail_serialize'
  LANGUAGE C IMMUTABLE STRICT;

/* deserialize data */
CREATE OR REPLACE FUNCTION funnel_detail_deserialize(p_value bytea, p_dummy internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'funnel_detail_deserialize'
  LANGUAGE C IMMUTABLE STRICT;


CREATE OR REPLACE FUNCTION funnel_result_reducer(integer[])
  RETURNS integer[]
  AS 'MODULE_PATHNAME','funnel_result_reducer'
  LANGUAGE C IMMUTABLE;

/* Create the aggregate functions */
/*-- funnel_count --*/
DROP AGGREGATE IF EXISTS funnel_count(integer, bigint, bigint, integer);
CREATE AGGREGATE funnel_count(total_step integer, window_size bigint, event_time bigint, step integer)
(
    SFUNC = funnel_detail_append,
    STYPE = internal,
    COMBINEFUNC = funnel_detail_combine,
    FINALFUNC = funnel_detail_reducer,
    SERIALFUNC = funnel_detail_serialize,
    DESERIALFUNC = funnel_detail_deserialize,
    PARALLEL = SAFE
);

/*--funnel_sum --*/
DROP AGGREGATE IF EXISTS funnel_sum(integer);
CREATE AGGREGATE funnel_sum (step integer)
(
    SFUNC = array_append,
    STYPE = integer[],
    FINALFUNC = funnel_result_reducer,
    INITCOND = '{}',
    PARALLEL = SAFE
);