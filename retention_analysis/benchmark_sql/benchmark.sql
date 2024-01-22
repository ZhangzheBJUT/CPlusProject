\timing on

\echo test query 10000
DROP TABLE IF EXISTS event_log_10000;
CREATE TABLE IF NOT EXISTS event_log_10000 (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    ds VARCHAR(32) NOT NULL
);
INSERT INTO event_log_10000 SELECT '5aa67c90f29d980c06000125' AS token,concat('99FF99ffb5f2a86D87d',floor(random()*1000)::text) AS distinct_id,(array['initial','return'])[floor(random()*2)::int + 1] AS event_name,(array['APP Store','AndroidPlay','Huawei'])[floor(random()*3)::int + 1] AS channel,floor(random()* (20200531-20200501 + 1) + 20200501)::text FROM generate_series(1,10000) s(i);

-- with newinst_tb(distinct_id_init, inst_date) as (
--       SELECT distinct_id, to_char(ds::date,'YYYYMMdd') as initial, '' as key
--       FROM event_log_10000
--       WHERE ds>='20200501' AND ds<='20200531' AND token ='5aa67c90f29d980c06000125' and event_name='initial'
-- ), return_tb(distinct_id_return, ret_date) as (
--      SELECT distinct_id, to_char(ds::date,'YYYYMMdd') as initial, '' as key
--      FROM event_log_10000
--      WHERE ds>='20200501' AND ds<='20200531' AND token ='5aa67c90f29d980c06000125' and event_name='return'
-- )
-- select inst_date,string_agg(cnt::text, ',' order by date)
-- from (
--      select inst_date, inst_date as date, count(distinct distinct_id_init) as cnt
--      from newinst_tb
--      group by inst_date, date
--      union all
--      select newinst_tb.inst_date, ret_date, count(distinct distinct_id_return) as cnt
--      from return_tb join newinst_tb
--      on (distinct_id_return = distinct_id_init and (
--       (to_date(ret_date,'YYYYMMdd')-to_date(inst_date,'YYYYMMdd')>0 AND (to_date(ret_date,'YYYYMMdd')-to_date(inst_date,'YYYYMMdd'))<=7 ) OR
--       (to_date(ret_date,'YYYYMMdd')-to_date(inst_date,'YYYYMMdd')=14)  OR
--       (to_date(ret_date,'YYYYMMdd')-to_date(inst_date,'YYYYMMdd')=30) ) )
--      group by inst_date, ret_date
-- ) t
-- group by inst_date
-- order by inst_date ;

\echo test query 1
SELECT retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds
    FROM event_log_10000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,ds
    UNION ALL
    SELECT 1 AS type,distinct_id,ds
    FROM event_log_10000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,ds
  )t1
  GROUP BY distinct_id
)t2;

\echo test query 2
SELECT channel,retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,channel,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds,channel
    FROM event_log_10000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,ds,channel
    UNION ALL
    SELECT 1 AS type,distinct_id,ds,channel
    FROM event_log_10000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,ds,channel
  )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;

--week
SELECT retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,retention_count(ds,'YYYYmmdd',type,'week') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,to_char(to_timestamp((extract(epoch from (to_timestamp(ds,'YYYYMMdd'))) - extract(DOW from to_timestamp(ds,'YYYYMMdd')) * 86400)),'YYYYMMdd') AS ds
    FROM event_log_10000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,to_char(to_timestamp((extract(epoch from (to_timestamp(ds,'YYYYMMdd'))) - extract(DOW from to_timestamp(ds,'YYYYMMdd')) * 86400)),'YYYYMMdd')
    UNION ALL
    SELECT 1 AS type,distinct_id,to_char(to_timestamp((extract(epoch from (to_timestamp(ds,'YYYYMMdd'))) - extract(DOW from to_timestamp(ds,'YYYYMMdd')) * 86400)),'YYYYMMdd') AS ds
    FROM event_log_10000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,to_char(to_timestamp((extract(epoch from (to_timestamp(ds,'YYYYMMdd'))) - extract(DOW from to_timestamp(ds,'YYYYMMdd')) * 86400)),'YYYYMMdd')
  )t1
  GROUP BY distinct_id
)t2;

--month
SELECT retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,retention_count(ds,'YYYYmmdd',type,'month') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,to_char(date_trunc('month',to_timestamp(ds,'YYYYMMdd')),'YYYYMMdd') AS ds
    FROM event_log_10000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,to_char(date_trunc('month',to_timestamp(ds,'YYYYMMdd')),'YYYYMMdd')
    UNION ALL
    SELECT 1 AS type,distinct_id,to_char(date_trunc('month',to_timestamp(ds,'YYYYMMdd')),'YYYYMMdd') AS ds
    FROM event_log_10000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,to_char(date_trunc('month',to_timestamp(ds,'YYYYMMdd')),'YYYYMMdd')
  )t1
  GROUP BY distinct_id
)t2;


\echo test query 100000
DROP TABLE IF EXISTS event_log_100000;
CREATE TABLE IF NOT EXISTS event_log_100000 (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    ds VARCHAR(32) NOT NULL
);
INSERT INTO event_log_100000 SELECT '5aa67c90f29d980c06000125' AS token,concat('99FF99ffb5f2a86D87d',floor(random()*10000)::text) AS distinct_id,(array['initial','return'])[floor(random()*2)::int + 1] AS event_name,(array['APP Store','AndroidPlay','Huawei'])[floor(random()*3)::int + 1] AS channel,floor(random()* (20200531-20200501 + 1) + 20200501)::text FROM generate_series(1,100000) s(i);

\echo test query 1
SELECT retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds
    FROM event_log_100000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,ds
    UNION ALL
    SELECT 1 AS type,distinct_id,ds
    FROM event_log_100000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,ds
  )t1
  GROUP BY distinct_id
)t2;

\echo test query 2
SELECT channel,retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,channel,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds,channel
    FROM event_log_100000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,ds,channel
    UNION ALL
    SELECT 1 AS type,distinct_id,ds,channel
    FROM event_log_100000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,ds,channel
  )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;



\echo test query 1000000
DROP TABLE IF EXISTS event_log_1000000;
CREATE TABLE IF NOT EXISTS event_log_1000000 (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    ds VARCHAR(32) NOT NULL
);
INSERT INTO event_log_1000000 SELECT '5aa67c90f29d980c06000125' AS token,concat('99FF99ffb5f2a86D87d',floor(random()*100000)::text) AS distinct_id,(array['initial','return'])[floor(random()*2)::int + 1] AS event_name,(array['APP Store','AndroidPlay','Huawei'])[floor(random()*3)::int + 1] AS channel,floor(random()* (20200531-20200501 + 1) + 20200501)::text FROM generate_series(1,1000000) s(i);

\echo test query 1
SELECT retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds
    FROM event_log_1000000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,ds
    UNION ALL
    SELECT 1 AS type,distinct_id,ds
    FROM event_log_1000000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,ds
  )t1
  GROUP BY distinct_id
)t2;

\echo test query 2
SELECT channel,retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,channel,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds,channel
    FROM event_log_1000000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,ds,channel
    UNION ALL
    SELECT 1 AS type,distinct_id,ds,channel
    FROM event_log_1000000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,ds,channel
  )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;



\echo test query 10000000
DROP TABLE IF EXISTS event_log_10000000;
CREATE TABLE IF NOT EXISTS event_log_10000000 (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    ds VARCHAR(32) NOT NULL
);
INSERT INTO event_log_10000000 SELECT '5aa67c90f29d980c06000125' AS token,concat('99FF99ffb5f2a86D87d',floor(random()*1000000)::text) AS distinct_id,(array['initial','return'])[floor(random()*2)::int + 1] AS event_name,(array['APP Store','AndroidPlay','Huawei'])[floor(random()*3)::int + 1] AS channel,floor(random()* (20200531-20200501 + 1) + 20200501)::text FROM generate_series(1,10000000) s(i);

\echo test query 1
SELECT retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds
    FROM event_log_10000000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,ds
    UNION ALL
    SELECT 1 AS type,distinct_id,ds
    FROM event_log_10000000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,ds
  )t1
  GROUP BY distinct_id
)t2;

\echo test query 2
SELECT channel,retention_sum(retention_state) AS result
FROM (
  SELECT distinct_id,channel,retention_count(ds,'YYYYmmdd',type,'day') AS retention_state
  FROM (
    SELECT 0 AS type,distinct_id,ds,channel
    FROM event_log_10000000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='initial'
    GROUP BY distinct_id,ds,channel
    UNION ALL
    SELECT 1 AS type,distinct_id,ds,channel
    FROM event_log_10000000
    WHERE ds>='20200501' AND ds<='20200531' AND token = '5aa67c90f29d980c06000125' AND event_name='return'
    GROUP BY distinct_id,ds,channel
  )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;

\timing off