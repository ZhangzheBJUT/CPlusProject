\timing on

\echo test query 10000
DROP TABLE IF EXISTS event_log_10000;
CREATE TABLE IF NOT EXISTS event_log_10000 (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    event_time BIGINT NOT NULL
);
INSERT INTO event_log_10000 SELECT '5aa67c90f29d980c06000125' AS token,concat('99FF99ffb5f2a86D87d',floor(random()*1000)::text) AS distinct_id,(array['step1','step2','step3'])[floor(random()*3)::int + 1] AS event_name,(array['APP Store','AndroidPlay','Huawei'])[floor(random()*3)::int + 1] AS channel, floor(random()* (1589299200000-1589212800000 + 1) + 1589212800000)::bigint  AS event_time FROM generate_series(1,10000) s(i);

-- with log(distinct_id,time, event_name) as (
--   select distinct_id,event_time as time,event_name from event_log_10000 where token ='5aa67c90f29d980c06000125'
-- )
-- select ARRAY[count(distinct step1.distinct_id) filter (where step1.time is not null),
-- count(distinct step2.distinct_id) filter (where step2.time is not null),
-- count(distinct step3.distinct_id) filter (where step3.time is not null)] from
-- (select distinct_id,time from log where event_name = 'step1') step1
-- left join
-- (select distinct_id,time from log where event_name = 'step2' ) step2
-- on (step1.distinct_id = step2.distinct_id and step1.time < step2.time and step2.time-step1.time <= 86400000)
-- left join
-- (select distinct_id,time from log where event_name = 'step3' ) step3
-- on (step1.distinct_id = step3.distinct_id and step2.time < step3.time and step3.time-step1.time <= 86400000);

-- with log(distinct_id,channel,time,event_name) as (
--   select distinct_id,channel,event_time as time,event_name from event_log_10000 where token ='5aa67c90f29d980c06000125'
-- )
-- select step1.channel,ARRAY[count(distinct step1.distinct_id) filter (where step1.time is not null AND step1.channel is not null),
-- count(distinct step2.distinct_id) filter (where step2.time is not null AND step2.channel is not null),
-- count(distinct step3.distinct_id) filter (where step3.time is not null AND step3.channel is not null)] from
-- (select distinct_id,channel,time from log where event_name = 'step1') step1
-- left join
-- (select distinct_id,channel,time from log where event_name = 'step2' ) step2
-- on (step1.distinct_id = step2.distinct_id and step1.channel = step2.channel and step1.time < step2.time and step2.time-step1.time <= 86400000)
-- left join
-- (select distinct_id,channel,time from log where event_name = 'step3' ) step3
-- on (step1.distinct_id = step3.distinct_id and step1.channel = step3.channel and step2.time < step3.time and step3.time-step1.time <= 86400000)
-- GROUP BY step1.channel;

SELECT funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_10000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id
)t2;

SELECT channel,funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,channel,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_10000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;

\echo test query 100000
DROP TABLE IF EXISTS event_log_100000;
CREATE TABLE IF NOT EXISTS event_log_100000 (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    event_time BIGINT NOT NULL
);
INSERT INTO event_log_100000 SELECT '5aa67c90f29d980c06000125' AS token,concat('99FF99ffb5f2a86D87d',floor(random()*10000)::text) AS distinct_id,(array['step1','step2','step3'])[floor(random()*3)::int + 1] AS event_name,(array['APP Store','AndroidPlay','Huawei'])[floor(random()*3)::int + 1] AS channel, floor(random()* (1589299200000-1589212800000 + 1) + 1589212800000)::bigint  AS event_time FROM generate_series(1,100000) s(i);

SELECT channel,funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,channel,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_100000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;

SELECT funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_100000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id
)t2;


\echo test query 1000000
DROP TABLE IF EXISTS event_log_1000000;
CREATE TABLE IF NOT EXISTS event_log_1000000 (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    event_time BIGINT NOT NULL
);
INSERT INTO event_log_1000000 SELECT '5aa67c90f29d980c06000125' AS token,concat('99FF99ffb5f2a86D87d',floor(random()*100000)::text) AS distinct_id,(array['step1','step2','step3'])[floor(random()*3)::int + 1] AS event_name,(array['APP Store','AndroidPlay','Huawei'])[floor(random()*3)::int + 1] AS channel, floor(random()* (1589299200000-1589212800000 + 1) + 1589212800000)::bigint  AS event_time FROM generate_series(1,1000000) s(i);

SELECT channel,funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,channel,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_1000000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;

SELECT funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_1000000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id
)t2;


\echo test query 10000000
DROP TABLE IF EXISTS event_log_10000000;
CREATE TABLE IF NOT EXISTS event_log_10000000 (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    event_time BIGINT NOT NULL
);
INSERT INTO event_log_10000000 SELECT '5aa67c90f29d980c06000125' AS token,concat('99FF99ffb5f2a86D87d',floor(random()*1000000)::text) AS distinct_id,(array['step1','step2','step3'])[floor(random()*3)::int + 1] AS event_name,(array['APP Store','AndroidPlay','Huawei'])[floor(random()*3)::int + 1] AS channel, floor(random()* (1589299200000-1589212800000 + 1) + 1589212800000)::bigint  AS event_time FROM generate_series(1,10000000) s(i);

SELECT channel,funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,channel,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_10000000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;

SELECT funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log_10000000
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id
)t2;

\timing off