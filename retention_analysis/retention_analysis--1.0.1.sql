/* retention_analysis */
CREATE OR REPLACE FUNCTION retention_detail_append( p_state internal, date_str text, format text, behavior_type int, granularity text)
  RETURNS internal
  AS 'MODULE_PATHNAME','retention_detail_append'
  LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION retention_detail_combine( p_state_1 internal, p_state_2 internal)
  RETURNS internal
  AS 'MODULE_PATHNAME','retention_detail_combine'
  LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION retention_detail_reducer(internal)
  RETURNS bigint[]
  AS 'MODULE_PATHNAME','retention_detail_reducer'
  LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION parse_retention_data(result bigint[])
  RETURNS text
AS 'MODULE_PATHNAME','parse_retention_data'
  LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION retention_offset_filter(result bigint[], n int)
  RETURNS int
AS 'MODULE_PATHNAME','retention_offset_filter'
  LANGUAGE C IMMUTABLE;

/* serialize data */
CREATE OR REPLACE FUNCTION retention_detail_serialize(p_pointer internal)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'retention_detail_serialize'
  LANGUAGE C IMMUTABLE STRICT;

/* deserialize data */
CREATE OR REPLACE FUNCTION retention_detail_deserialize(p_value bytea, p_dummy internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'retention_detail_deserialize'
  LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION retention_result_reducer(bigint[])
  RETURNS text[]
  AS 'MODULE_PATHNAME','retention_result_reducer'
  LANGUAGE C IMMUTABLE;


/* for result sum*/
CREATE OR REPLACE FUNCTION retention_result_append( p_state internal, detail_result bigint[])
  RETURNS internal
AS 'MODULE_PATHNAME','retention_result_append'
  LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION retention_result_combine( p_state_1 internal, p_state_2 internal)
  RETURNS internal
AS 'MODULE_PATHNAME','retention_result_combine'
  LANGUAGE C IMMUTABLE;

CREATE OR REPLACE FUNCTION retention_result_serialize(p_pointer internal)
  RETURNS bytea
AS 'MODULE_PATHNAME', 'retention_result_serialize'
  LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION retention_result_deserialize(p_value bytea, p_dummy internal)
  RETURNS internal
AS 'MODULE_PATHNAME', 'retention_result_deserialize'
  LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION retention_result_reducer_v1(internal)
  RETURNS text[]
AS 'MODULE_PATHNAME','retention_result_reducer_v1'
  LANGUAGE C IMMUTABLE;


/* Create the aggregate functions */
/*-- retention_count --*/
DROP AGGREGATE IF EXISTS retention_count(text,text,int,text);
CREATE AGGREGATE retention_count(date_str text, format text, behavior_type int, granularity text)
(
    SFUNC = retention_detail_append,
    STYPE = internal,
    COMBINEFUNC = retention_detail_combine,
    FINALFUNC = retention_detail_reducer,
    SERIALFUNC = retention_detail_serialize,
    DESERIALFUNC = retention_detail_deserialize,
    PARALLEL = SAFE
);

/*--retention_sum --*/
DROP AGGREGATE IF EXISTS retention_sum(bigint[]);
CREATE AGGREGATE retention_sum (retention_data bigint[])
(
  SFUNC = retention_result_append,
  STYPE = internal,
  COMBINEFUNC = retention_result_combine,
  SERIALFUNC = retention_result_serialize,
  DESERIALFUNC = retention_result_deserialize,
  FINALFUNC = retention_result_reducer_v1,
  PARALLEL = SAFE
);

/*--retention_sum_bak(not used) --*/
DROP AGGREGATE IF EXISTS retention_sum_bak(bigint[]);
CREATE AGGREGATE retention_sum_bak (retention_data bigint[])
(
  SFUNC = array_cat,
  STYPE = bigint[],
  FINALFUNC = retention_result_reducer,
  INITCOND = '{}',
  PARALLEL = SAFE
);