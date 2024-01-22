DROP TABLE IF EXISTS event_log;
CREATE TABLE IF NOT EXISTS event_log (
    token VARCHAR(48) NOT NULL,
    distinct_id VARCHAR(48) NOT NULL,
    event_name VARCHAR(32) NOT NULL,
    channel VARCHAR(32) NOT NULL,
    event_time BIGINT NOT NULL
);

INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19A','step2','XiaoMi',1583475650000);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19B','step1','XiaoMi',1583475660000);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19B','step2','XiaoMi',1583475660001);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19B','step3','XiaoMi',1583475660002);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19B','step4','XiaoMi',1583475660003);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c18B','step1','XiaoMi',1583475660000);

INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19C','step1','HuaWei',1583475660000);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19C','step2','HuaWei',1583475660001);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19C','step3','HuaWei',1583475660002);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19C','step4','HuaWei',1583475660003);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19D','step1','HuaWei',1583475660001);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19E','step1','HuaWei',1583475660002);
INSERT INTO event_log VALUES('5aa67c90f29d980c06000125','99FF99ffb5f2a86D87d2e686EF93c19E','step2','HuaWei',1583475660003);


-- \echo test query 1
SELECT distinct_id,channel,funnel_count(3,86400000,event_time,step) AS funnel_state
  FROM (
    SELECT distinct_id,channel,event_time,
           CASE WHEN event_name='step1' THEN 1
                WHEN event_name='step2' THEN 2
                WHEN event_name='step3' THEN 3
           END AS step
    FROM event_log
    WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
  )t1
GROUP BY distinct_id,channel;


\echo test query 2
SELECT channel,funnel_sum(funnel_state)
FROM (
  SELECT distinct_id,channel,funnel_count(3,86400000,event_time,step) AS funnel_state
    FROM (
      SELECT distinct_id,channel,event_time,
             CASE WHEN event_name='step1' THEN 1
                  WHEN event_name='step2' THEN 2
                  WHEN event_name='step3' THEN 3
             END AS step
      FROM event_log
      WHERE token = '5aa67c90f29d980c06000125' AND event_name IN ('step1','step2','step3')
    )t1
  GROUP BY distinct_id,channel
)t2
GROUP BY channel;